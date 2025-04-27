#include "CombatSystem/HitReactionSystem/HitReactionComponent.h"

#include "AssetTypeActions/AssetDefinition_SoundBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"

UHitReactionComponent::UHitReactionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UHitReactionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UHitReactionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsPushingBack)
	{
		AActor* Owner = GetOwner();
		if (!IsValid(Owner))
		{
			bIsPushingBack = false;
			return;
		}

		FVector CurrentLocation = Owner->GetActorLocation();
		FVector NewLocation = FMath::VInterpTo(CurrentLocation, PushTargetLocation, DeltaTime, PushInterpSpeed);
		Owner->SetActorLocation(NewLocation, true);

		float DistanceToTarget = FVector::Dist(NewLocation, PushTargetLocation);

		if (DistanceToTarget < 1.0f)
		{
			Owner->SetActorLocation(PushTargetLocation, true);
			bIsPushingBack = false;
		}
	}
}

void UHitReactionComponent::SetHitReaction_Implementation(bool InIsAirLaunchAttack, float InDamage, bool InHitPushBack, float InPushBackDistance, USoundBase* InHitSFX, UNiagaraSystem* InHitParticle, TSubclassOf<UCameraShakeBase> InCameraShake, float InCameraShakeScale)
{
	bIsAirLaunchAttack = InIsAirLaunchAttack;
	Damage = InDamage;
	bHitPushBack = InHitPushBack;
	PushBackDistance = InPushBackDistance;
	HitSFX = InHitSFX;
	HitParticleFX = InHitParticle;
	CameraShake = InCameraShake;
	CameraShakeScale = InCameraShakeScale;
}

void UHitReactionComponent::OnHitReaction(const float InDamage, AActor* InDamageCauser, const FHitResult& InHitInfo, FName InDamagedActorTag)
{
	if (!IsValid(InDamageCauser) || InDamageCauser->ActorHasTag(InDamagedActorTag))
		return;

	if (!bIsAlive)
		return;

	DamageCauser = InDamageCauser;
	UHitReactionComponent* DamageCauserHR = InDamageCauser->FindComponentByClass<UHitReactionComponent>();
	if (!IsValid(DamageCauserHR))
		return;

	if (DamageCauserHR->bHitPushBack)
		PushBack(DamageCauserHR->PushBackDistance);
	
	HitInfo = InHitInfo;
	UAnimMontage* HitMontage = GetHitMontage();
	HitFeedBackAnim(HitMontage, GetOwner());
	HitFeedBackFX();	

	Health -= InDamage;
	if (Health <= 0)
	{
		RagdollEvent();
		OnDeath();
	}
}

UAnimMontage* UHitReactionComponent::GetHitMontage()
{
	if (!IsValid(DamageCauser))
		return nullptr;
		
	UHitReactionComponent* DamageCauserHR = DamageCauser->FindComponentByClass<UHitReactionComponent>();
	if (!IsValid(DamageCauserHR))
		return nullptr;

	if (bIsAirLaunchAttack)
		return AirLaunchReactionMontage;

	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
		return nullptr;

	UCharacterMovementComponent* CharacterMovementComponent = Owner->GetComponentByClass<UCharacterMovementComponent>();
	if (!IsValid(CharacterMovementComponent))
		return nullptr;

	if (CharacterMovementComponent->IsFalling() || CharacterMovementComponent->IsFlying())
		return AirHitReactionMontage;

	return IsBackHit() ? FrontHitReactionMontage : BackHitReactionMontage;
}

void UHitReactionComponent::HitFeedBackAnim(UAnimMontage* Montage, const AActor* TargetActor)
{
	if (!IsValid(TargetActor) || !IsValid(Montage))
		return;

	
	const USkeletalMeshComponent* SkeletalMesh = TargetActor->GetComponentByClass<USkeletalMeshComponent>();
	if (!IsValid(SkeletalMesh))
		return;
		
	UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance();
	if (IsValid(AnimInstance))
		AnimInstance->Montage_Play(Montage);
	UE_LOG(LogTemp, Warning, TEXT("Montage: %s"), *Montage->GetName());	
}

void UHitReactionComponent::HitFeedBackFX()
{
	if (!IsValid(DamageCauser))
		return;
	
	UHitReactionComponent* DamageCauserHR = DamageCauser->FindComponentByClass<UHitReactionComponent>();
	if (!IsValid(DamageCauserHR))
		return;

	// Sound FX
	UGameplayStatics::PlaySoundAtLocation(this, DamageCauserHR->HitSFX, HitInfo.ImpactPoint);
	// Particle FX
	//UGameplayStatics::SpawnEmitterAtLocation(this, DamageCauserHR->HitParticleFX, HitInfo.ImpactPoint, UKismetMathLibrary::MakeRotFromX(HitInfo.ImpactPoint));
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, DamageCauserHR->HitParticleFX, HitInfo.ImpactPoint, UKismetMathLibrary::MakeRotFromX(HitInfo.ImpactPoint));
}

void UHitReactionComponent::PushBack(const float InPushBackDistance)
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !IsValid(DamageCauser))
	{
		return;
	}

	// Push Location
	const FVector OwnerLocation = Owner->GetActorLocation();
	const FVector DamageCauserLocation = DamageCauser->GetActorLocation();
	FVector PushDirection = (OwnerLocation - DamageCauserLocation).GetSafeNormal();

	FVector FinalLocation = OwnerLocation + PushDirection * InPushBackDistance;
	bIsPushingBack = true;
	PushTargetLocation = FinalLocation;

	// Push Rotation
	FRotator FinalRotation = (IsBackHit())
		? FRotator(0.0f, UKismetMathLibrary::FindLookAtRotation(OwnerLocation, DamageCauserLocation).Yaw, 0.0f)
		: Owner->GetActorRotation();

	Owner->SetActorRotation(FinalRotation);
}

bool UHitReactionComponent::IsBackHit()
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !IsValid(DamageCauser))
		return false;

	const FRotator OwnerRotation = Owner->GetActorRotation();
	const FRotator DirectionToCauser = UKismetMathLibrary::FindLookAtRotation(Owner->GetActorLocation(), DamageCauser->GetActorLocation());
	const FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(OwnerRotation, DirectionToCauser);

	const float Yaw = DeltaRotation.Yaw;
	
	bool bIsBackHit = FMath::Abs(Yaw) >= 110.f && FMath::Abs(Yaw) <= 180.f;
	
	return !bIsBackHit;
}

void UHitReactionComponent::DoCameraShake()
{
	APlayerController* PlayerController = Cast<APlayerController>(DamageCauser->GetInstigator()->GetController());
	if (!IsValid(DamageCauser) || !IsValid(PlayerController))
		return;
	
	UHitReactionComponent* DamageCauserHR = DamageCauser->FindComponentByClass<UHitReactionComponent>();
	if (!IsValid(DamageCauserHR))
		return;
		
	PlayerController->ClientStartCameraShake(DamageCauserHR->CameraShake, DamageCauserHR->CameraShakeScale, ECameraShakePlaySpace::CameraLocal);	
}

void UHitReactionComponent::RagdollEvent()
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
		return;

	UCharacterMovementComponent* CharacterMovementComponent = Owner->GetComponentByClass<UCharacterMovementComponent>();
	if (!IsValid(CharacterMovementComponent))
		return;

	USkeletalMeshComponent* SkeletalMeshComponent = Owner->GetComponentByClass<USkeletalMeshComponent>();
	if (!IsValid(SkeletalMeshComponent))
		return;
	
	UCapsuleComponent* CapsuleComponent = Owner->GetComponentByClass<UCapsuleComponent>();
	if (!IsValid(CapsuleComponent))
		return;

	CharacterMovementComponent->SetMovementMode(MOVE_None);
	
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SkeletalMeshComponent->SetCollisionObjectType(ECC_PhysicsBody);
	SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SkeletalMeshComponent->SetAllBodiesBelowSimulatePhysics(FName(TEXT("Pelvis")), true, true);

	UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
	if (IsValid(AnimInstance))
		AnimInstance->Montage_Stop(0.25f);

	/*
	if (HitInfo.bBlockingHit)
	{
		FVector ImpulseVector = (HitInfo.TraceEnd - HitInfo.TraceStart) * 100.0f;
		SkeletalMeshComponent->AddImpulseAtLocation(ImpulseVector, HitInfo.ImpactPoint, HitInfo.BoneName);
	}
	*/
}

void UHitReactionComponent::OnDeath()
{
	bIsAlive = false;
}

void UHitReactionComponent::RemoveStamina(const float Amount)
{
	if (Amount <= 0.f)
		return;
	Stamina = FMath::Clamp(Stamina - Amount, 0.0f, MaxStamina);

	if (Stamina < MaxStamina)
	{
		GetWorld()->GetTimerManager().SetTimer(StaminaRegenHandle, this, &UHitReactionComponent::RegenStamina, StaminaRegenPerSec, true);
	}
}

void UHitReactionComponent::RegenStamina()
{
	if (!bIsAlive)
		return;

	if (Stamina >= MaxStamina)
	{
		GetWorld()->GetTimerManager().ClearTimer(StaminaRegenHandle);
		return;
	}

	Stamina++;
}

void UHitReactionComponent::LaunchCharacter(float LaunchVelocityZ)
{
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!IsValid(Owner))
		return;

	UE_LOG(LogTemp, Log, TEXT("OK"));

	Owner->LaunchCharacter(FVector(0.0f, 0.0f, LaunchVelocityZ), true, true);
}
