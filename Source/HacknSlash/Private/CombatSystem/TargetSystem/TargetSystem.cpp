#include "CombatSystem/TargetSystem/TargetSystem.h"

#include "CombatSystem/HitReactionSystem/HitReactionComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "WorldPartition/ContentBundle/ContentBundleLog.h"
#include "CombatSystem/TargetSystem/TargetSystemInterface.h"
#include "Components/WidgetComponent.h"

UTargetSystem::UTargetSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UTargetSystem::BeginPlay()
{
	Super::BeginPlay();
	
}

void UTargetSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UTargetSystem::LockOnTarget()
{
	if (bIsTarget)
	{
		Reset();
		return;
	}

	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
		return;
	
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(Owner);
	IgnoreActors.Add(TargetActor);

	TArray<FHitResult> OutHits;
	bool bIsHitTarget = FindTargetsInRadius(FindTargetRadius, IgnoreActors, ECC_Pawn, OutHits);

	if (!bIsHitTarget)
	{
		if (bIsTarget)
			Reset();
	}

	for (const FHitResult& Hit: OutHits)
	{
		if (!IsValid(Hit.GetActor()))
			continue;
		
		UHitReactionComponent* HitReaction = Hit.GetActor()->FindComponentByClass<UHitReactionComponent>();
		if (!IsValid(HitReaction))
			continue;

		if (!HitReaction->IsAlive() || !Hit.GetActor()->ActorHasTag(FName("Enemy")))
			continue;

		TargetActors.AddUnique(Hit.GetActor());
		bIsTarget = true;
	}

	GetNearestTarget();	
}

bool UTargetSystem::FindTargetsInRadius(const float RadiusToFind, const TArray<AActor*>& ActorsToIgnore, ECollisionChannel ObjectTypeQuery, TArray<FHitResult>& OutHits)
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
		return false;

	FVector StartLocation = GetOwner()->GetActorLocation();
	FVector EndLocation = StartLocation + Owner->GetActorForwardVector();
	
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	
	bool IsHit = UKismetSystemLibrary::SphereTraceMultiForObjects(
		GetWorld(),
		StartLocation,
		EndLocation,
		RadiusToFind,
		ObjectTypes, 
		false,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		OutHits,
		true
		);

	return IsHit;
}

void UTargetSystem::GetNearestTarget()
{
	if (!bIsTarget)
		return;

	if (IsValid(TargetActor) && TargetActor->Implements<UTargetSystemInterface>())
	{
		UWidgetComponent* TargetWidget = ITargetSystemInterface::Execute_GetWidgetTargetComponent(TargetActor);
		if (IsValid(TargetWidget))
		{
			TargetWidget->SetHiddenInGame(true);
		}
	}

	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
		return;

	FVector OwnerLocation = GetOwner()->GetActorLocation();
	
    if (TargetActors.IsEmpty())
    {
        TargetActor = nullptr;
        return;
    }

    float NearestDistanceSq = FLT_MAX;
    AActor* NewTargetActor = nullptr;

    for (AActor* CandidateActor : TargetActors)
    {
        if (!IsValid(CandidateActor))
            continue;

        const float CurrentDistanceSq = FVector::DistSquared(OwnerLocation, CandidateActor->GetActorLocation());
        if (CurrentDistanceSq < NearestDistanceSq)
        {
            NearestDistanceSq = CurrentDistanceSq;
            NewTargetActor = CandidateActor;
        }
    }

	TargetActor = NewTargetActor;

	if (!IsValid(TargetActor))
		return;

	if (TargetActor->Implements<UTargetSystemInterface>())
	{
		UWidgetComponent* TargetWidget = ITargetSystemInterface::Execute_GetWidgetTargetComponent(TargetActor);
		if (IsValid(TargetWidget))
		{
			TargetWidget->SetHiddenInGame(false);
		}
	}

	// Tracking on Target
	FTimerHandle TimerHandle;
    if (!GetWorld()->GetTimerManager().IsTimerActive(TimerHandle))
    {
        GetWorld()->GetTimerManager().SetTimer(
            TimerHandle,
            this,
			&UTargetSystem::TrackTarget,
            0.01f,
            true
        );
    }	
}

void UTargetSystem::NextNearestTarget()
{
	if (!bIsTarget)
		return;
	TargetActors.Empty();

	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
		return;
	
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(Owner);
	IgnoreActors.Add(TargetActor);

	TArray<FHitResult> OutHits;
	bool bIsHitTarget = FindTargetsInRadius(FindTargetRadius, IgnoreActors, ECC_Pawn, OutHits);
	
	if (!bIsHitTarget)
	{
		if (bIsTarget)
			Reset();
	}

	for (const FHitResult& Hit: OutHits)
	{
		if (!IsValid(Hit.GetActor()))
			continue;
		
		UHitReactionComponent* HitReaction = Hit.GetActor()->FindComponentByClass<UHitReactionComponent>();
		if (!IsValid(HitReaction))
			continue;

		if (!HitReaction->IsAlive() || !Hit.GetActor()->ActorHasTag(FName("Enemy")))
			continue;

		TargetActors.AddUnique(Hit.GetActor());
		bIsTarget = true;
	}

	GetNearestTarget();	
}


void UTargetSystem::Reset()
{
	bIsTarget = false;

	if (IsValid(TargetActor) && TargetActor->Implements<UTargetSystemInterface>())
	{
		UWidgetComponent* TargetWidget = ITargetSystemInterface::Execute_GetWidgetTargetComponent(TargetActor);
		if (IsValid(TargetWidget))
		{
			TargetWidget->SetHiddenInGame(true);
		}
	}
	
	TargetActor	= nullptr;
	TargetActors.Empty();
}

void UTargetSystem::ApproachToTarget(float RequiredDistanceToFaceTarget, float RequiredDistanceToMoveToTarget, bool bSwitchToNearestEnemy, float ApproachTargetOffset, float InterpSpeed, bool bMoveToTarget)
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !bIsTarget || !IsValid(TargetActor))
		return;
	
	float DistanceToTarget = Owner->GetDistanceTo(TargetActor);
	if (DistanceToTarget <= RequiredDistanceToFaceTarget)
	{
		// Rotate to
		//FRotator OwnerRotation = Owner->GetActorRotation();
		//FRotator DesiredRotation = FRotator(OwnerRotation.Pitch, LookAtTargetRotation.Yaw, OwnerRotation.Roll);
		//FRotator FinalRotation = UKismetMathLibrary::RInterpTo(OwnerRotation, DesiredRotation, GetWorld()->GetDeltaSeconds(), InterpSpeed);

		FVector OwnerLocation = Owner->GetActorLocation();
		FVector TargetLocation = TargetActor->GetActorLocation();

		FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(OwnerLocation, TargetLocation);

		FRotator OwnerRotation = Owner->GetActorRotation();
		FRotator DesiredRotation = FRotator(OwnerRotation.Pitch, LookAtRotation.Yaw, OwnerRotation.Roll);

		FRotator FinalRotation = UKismetMathLibrary::RInterpTo(OwnerRotation, DesiredRotation, GetWorld()->GetDeltaSeconds(), InterpSpeed);
		
		Owner->SetActorRotation(FinalRotation);
	}
	if (DistanceToTarget <= RequiredDistanceToMoveToTarget)
	{
		if (bMoveToTarget)
		{
			// Move to
			//FRotator OwnerRotation = Owner->GetActorRotation();
			//FRotator DesiredRotation = FRotator(OwnerRotation.Pitch, LookAtTargetRotation.Yaw, OwnerRotation.Roll);
			//FVector DesiredLocaiton = TargetActor->GetActorLocation() + (UKismetMathLibrary::GetForwardVector(DesiredRotation) * ApproachTargetOffset);

			FVector OwnerLocation = Owner->GetActorLocation();
			FVector TargetLocation = TargetActor->GetActorLocation();

			FVector ToOwnerDirection = (OwnerLocation - TargetLocation).GetSafeNormal();
			FVector DesiredLocation = TargetLocation + ToOwnerDirection * ApproachTargetOffset;
			
			FVector FinalLocation = UKismetMathLibrary::VInterpTo(OwnerLocation, DesiredLocation, GetWorld()->GetDeltaSeconds(), InterpSpeed);
			
			Owner->SetActorLocation(FinalLocation);
		}
	}
	else if(bSwitchToNearestEnemy)
	{
		// TODO
	}
}

void UTargetSystem::SetLockedOnTarget(AActor* InLockedOnTarget)
{
	LockedOnTarget = InLockedOnTarget;
}

void UTargetSystem::TrackTarget()
{
	if (!bIsTarget)
		return;

	if (!IsValid(TargetActor))
		return;

	UHitReactionComponent* HitReaction = TargetActor->FindComponentByClass<UHitReactionComponent>();
	if (HitReaction->IsAlive())
	{
		AActor* Owner = GetOwner();
		if (!IsValid(Owner))
			return;
		float DistanceToTarget = Owner->GetDistanceTo(TargetActor);
		if (DistanceToTarget <= MaxTargetDistance)
		{
			// TODO: Look At Target Modes
		}
		else
		{
			// If target is too far, reset target
			Reset();
		}
	}
	else
	{
		NextNearestTarget();
	}
}