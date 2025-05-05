#include "CombatSystem/TargetSystem/TargetSystem.h"

#include "CombatSystem/HitReactionSystem/HitReactionComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "WorldPartition/ContentBundle/ContentBundleLog.h"

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

	//TargetActors.RemoveAll([](AActor* Actor) { return !IsValid(Actor); });
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
		EDrawDebugTrace::ForDuration,
		OutHits,
		true
		);

	return IsHit;
}

void UTargetSystem::GetNearestTarget()
{
	if (!bIsTarget)
		return;

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
	if (IsValid(TargetActor))
	{
		LookAtTargetRotation = UKismetMathLibrary::FindLookAtRotation(OwnerLocation, TargetActor->GetActorLocation());
	}
}

void UTargetSystem::Reset()
{
	bIsTarget = false;
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


