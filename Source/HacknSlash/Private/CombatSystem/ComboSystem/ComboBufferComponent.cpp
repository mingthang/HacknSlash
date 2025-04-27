#include "CombatSystem/ComboSystem/ComboBufferComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Player/PlayerCharacter.h"
#include "CombatSystem/HitboxSystem/HitboxInterface.h"
#include "CombatSystem/HitboxSystem/HitboxComponent.h"
#include "CombatSystem/HitReactionSystem/HitReactionComponent.h"

UComboBufferComponent::UComboBufferComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UComboBufferComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UComboBufferComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UComboBufferComponent::InitWeaponStates(USkeletalMeshComponent* InSkeletalMeshComponent)
{
	SkeletalMeshComponent = InSkeletalMeshComponent;

	if (!WeaponProfileTable)
    {
    	UE_LOG(LogTemp, Error, TEXT("WeaponProfileTable is nullptr! Please assign it in BP or code."));
    	return;
    }
	
	// TODO: Get Inventory CurrentWeapons -> Get it's Data in DT_WeaponProfiles -> Spawn Weapon Actor
	// -> Store in SpawnedWeapons -> Attach to SkeletonMeshComponent
	for (const auto& StoredWeaponName : CurrentWeapons)
	{
		const FWeaponProfile* Found = WeaponProfileTable->FindRow<FWeaponProfile>(
			StoredWeaponName,
			TEXT("LoadWeaponProfileFromTable")
		);
		if (!Found)
		{
			UE_LOG(LogTemp, Warning, TEXT("UComboBufferComponent::InitWeaponStates Missing Weapon Profile DataTable or cannot find StoredWeapon."));
			continue;
		}

		// Spawn
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = GetOwner();
		UClass* WeaponClass = AWeapon::StaticClass();
		AWeapon* SpawnedWeapon = GetWorld()->SpawnActor<AWeapon>(WeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
		if (SpawnedWeapon)
		{
			SpawnedWeapon->InitWeapon(StoredWeaponName, *Found);
			SpawnedWeapons.Add(SpawnedWeapon);
			
			FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
			SpawnedWeapon->AttachToComponent(SkeletalMeshComponent, AttachRules, SpawnedWeapon->WeaponProfile.WeaponData.WeaponSocket);
		}
	}
	
	CurrentWeaponIndex = INDEX_NONE;
	EquipWeapon(0);
}

void UComboBufferComponent::EquipWeapon(int32 WeaponIndex)
{
	if (!(CurrentWeapons.IsValidIndex(WeaponIndex) && CurrentWeaponIndex != WeaponIndex))
		return;

	// UnEquip Existing Weapon
	if (IsValid(CurrentWeapon) && IsValid(CurrentWeapon->WeaponProfile.ActionsData.HolsterMontage))
	{
		float MontageLength = PlayAnimMontage(CurrentWeapon->WeaponProfile.ActionsData.HolsterMontage, 1.0f);

		FTimerHandle EquipDelayHandle;
		FTimerDelegate EquipDelegate;

		EquipDelegate.BindLambda([this, WeaponIndex] ()
		{
			CurrentWeaponIndex = WeaponIndex;
			CurrentWeapon = SpawnedWeapons[CurrentWeaponIndex];

			if (IsValid(CurrentWeapon) && IsValid(CurrentWeapon->WeaponProfile.ActionsData.HolsterMontage))
			{
				PlayAnimMontage(CurrentWeapon->WeaponProfile.ActionsData.HolsterMontage, 1.0f);
			}
		});
		
		GetWorld()->GetTimerManager().SetTimer(EquipDelayHandle, EquipDelegate, MontageLength, false);
	}

	CurrentWeaponIndex = WeaponIndex;
	CurrentWeapon = SpawnedWeapons[CurrentWeaponIndex];
	//
	
	UHitboxComponent* HitboxComponent = GetOwner()->FindComponentByClass<UHitboxComponent>();
	if (HitboxComponent && HitboxComponent->GetClass()->ImplementsInterface(UHitboxInterface::StaticClass()))
    {
		UPrimitiveComponent* WeaponMeshComp = Cast<UPrimitiveComponent>(CurrentWeapon->GetComponentByClass(UStaticMeshComponent::StaticClass()));
    	IHitboxInterface::Execute_InitMesh(HitboxComponent, WeaponMeshComp);
    }
	
	if (IsValid(CurrentWeapon) && IsValid(CurrentWeapon->WeaponProfile.ActionsData.HolsterMontage))
	{
		PlayAnimMontage(CurrentWeapon->WeaponProfile.ActionsData.HolsterMontage, 1.0f);
	}
}

float UComboBufferComponent::PlayAnimMontage(UAnimMontage* Montage, float PlayRate)
{
	if (!SkeletalMeshComponent || !Montage) return 0.0f;
	
	if (UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance())
	{
		float MontageLength = AnimInstance->Montage_Play(Montage, PlayRate);

		if (IsInAir())
		{
			SetMovementMode(MOVE_Flying);
			float DelayTime = MontageLength * 0.8f;
			
			FTimerHandle TimerHandle;
			FTimerDelegate TimerDel;

			TimerDel.BindLambda([this] ()
			{
				if (IsInAir())
					SetMovementMode(MOVE_Falling);
			});
			
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, DelayTime, false);
		}

		return MontageLength;
	}

	return 0.0f;
}

void UComboBufferComponent::OnComboButtonDown(EComboInput Input)
{
	CurrentInputStartTime = GetWorld()->GetTimeSeconds();
	SetComboInput(true, Input);
}

void UComboBufferComponent::OnComboButtonUp(EComboInput Input)
{
	LastInputTimeUp = GetWorld()->GetTimeSeconds();
}

void UComboBufferComponent::SetComboInput(bool IsPressed, EComboInput ComboInput)
{
	if (!IsPressed || !CurrentWeapon)
		return;

	switch (CurrentComboState)
	{
	case EComboState::Idle:
		StartNewComboChain(ComboInput);
		break;
	case EComboState::Executing:
		// ignore until window opens
		break;
	case EComboState::WaitingForInput:
		BufferComboInput(ComboInput);
		break;
	}
}

void UComboBufferComponent::StartNewComboChain(EComboInput Input)
{
	bComboInProgress  = true;
	CurrentComboIndex = 0;
	CurrentComboTag   = GetCurrentComboTag();
	QueueChainedCombo(Input);
}

void UComboBufferComponent::QueueChainedCombo(EComboInput Input)
{
	if (!bComboInProgress || !CurrentWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("⛔ Combo not in progress or CurrentWeapon is null."));
		return;
	}

	int32 NumChainedCombos = CurrentWeapon->WeaponProfile.ComboSet.ChainedCombos.Num();
	int32 SelectedIdx = -1;
	for (int32 ChainedCombosIdx  = 0; ChainedCombosIdx < NumChainedCombos; ++ChainedCombosIdx)
	{
		EComboTag ChainedComboTag = CurrentWeapon->WeaponProfile.ComboSet.ChainedCombos[0].ComboTag;
		if (ChainedComboTag == CurrentComboTag)
		{
			SelectedIdx = ChainedCombosIdx;
			break;
		}
	}

	if (SelectedIdx == -1)
		return;
	
	const auto& Steps = CurrentWeapon->WeaponProfile.ComboSet.ChainedCombos[SelectedIdx].ChainedCombo.Steps;
	if (CurrentComboIndex >= Steps.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("⛔ Combo index %d out of range (Max = %d). Combo ended."), CurrentComboIndex, Steps.Num());
		bComboInProgress = false;
		CurrentComboState = EComboState::Idle;
		return;
	}

	const FChainedComboStep& Step = Steps[CurrentComboIndex];
	int8  Dir   = LastInputDirection;
	float Hold  = GetCurrentHoldDuration();

	UE_LOG(LogTemp, Warning, TEXT("🔁 Step [%d] — Checking %d branches | Input=%d | Delta=%.2f | Dir=%d | Hold=%.2f"),
		CurrentComboIndex,
		Step.Branches.Num(),
		static_cast<int32>(Input),
		Delta,
		Dir,
		Hold
	);

	// Duyệt qua các branch trong step
	for (int32 BranchIndex = 0; BranchIndex < Step.Branches.Num(); BranchIndex++)
	{
		const FComboBranch& B = Step.Branches[BranchIndex];
		UE_LOG(LogTemp, Warning, TEXT(">>> Checking Branch: Input=%d | MinDelta=%.2f | MaxDelta=%.2f | Dir=%d | Hold=%.2f | HoldThresh=%.2f | bRequireHold=%s"),
			static_cast<int32>(B.ComboInput),
			B.MinDeltaTime,
			B.MaxDeltaTime,
			B.Direction,
			Hold,
			B.HoldThreshold,
			B.bRequireHold ? TEXT("true") : TEXT("false")
		);
		if (B.ComboInput != Input)
		{
			UE_LOG(LogTemp, Warning, TEXT("⛔ Skip: ComboInput doesn't match. Expected: %d | Got: %d"), static_cast<int32>(B.ComboInput), static_cast<int32>(Input));
			continue;
		}
		if (Delta < B.MinDeltaTime || Delta > B.MaxDeltaTime)
		{
			UE_LOG(LogTemp, Warning, TEXT("⛔ Skip: Delta time %.2f out of range [%.2f, %.2f]"), Delta, B.MinDeltaTime, B.MaxDeltaTime);
			continue;
		}
		if (B.Direction != 0 && B.Direction != Dir)
		{
			UE_LOG(LogTemp, Warning, TEXT("⛔ Skip: Direction mismatch. Required: %d | Got: %d"), B.Direction, Dir);
			continue;
		}
		if (B.bRequireHold && Hold < B.HoldThreshold)
		{
			UE_LOG(LogTemp, Warning, TEXT("⛔ Skip: Hold time %.2f < threshold %.2f (RequireHold=true)"), Hold, B.HoldThreshold);
			continue;
		}
		if (!B.bRequireHold && Hold >= B.HoldThreshold)
		{
			UE_LOG(LogTemp, Warning, TEXT("⛔ Skip: Hold time %.2f >= threshold %.2f (RequireHold=false)"), Hold, B.HoldThreshold);
			continue;
		}
		if (B.RequiresPreviousBranch != -1 &&
            B.RequiresPreviousBranch != LastBranchIndex)
        {
			UE_LOG(LogTemp, Warning, TEXT("⛔ Skip: Require=%d LastBranchIndex=%d"), B.RequiresPreviousBranch, LastBranchIndex);
            continue;
        }
		
		UE_LOG(LogTemp, Warning, TEXT("✅ Branch matched! Proceeding with this combo path."));
		UE_LOG(LogTemp, Warning, TEXT("🎬 Montage: %s"), *GetNameSafe(B.Montage));

		// Hitbox notify montage start time
		for (const FAnimNotifyEvent& NotifyEvent : B.Montage->Notifies)
		{
			if (NotifyEvent.NotifyStateClass && NotifyEvent.NotifyStateClass->GetFName() == TEXT("BP_ANS_Melee_Hitbox_C_0"))
			{
				UHitboxComponent* HitboxComponent = GetOwner()->FindComponentByClass<UHitboxComponent>();
				if (HitboxComponent)
				{
					HitboxComponent->SetMontageTime(NotifyEvent.GetTime());
				}
			}
		}
		
		PlayAnimMontage(B.Montage, 1.0f);
		CurrentComboState = EComboState::Executing;
		CurrentComboIndex++;
		LastBranchIndex = BranchIndex;
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("❌ No valid branch matched. Combo ends."));
	bComboInProgress = false;
	CurrentComboState       = EComboState::Idle;
}

void UComboBufferComponent::BufferComboInput(EComboInput Input)
{
	if (!BufferedInput.IsSet())
	{
		BufferedInput = Input;
		Delta = GetWorld()->GetTimeSeconds() - InputWindowStartTime;
		UE_LOG(LogTemp, Warning, TEXT(">> BufferedInput = %d (Delta=%.2f)"),
			static_cast<int32>(Input), Delta);
	}
}

void UComboBufferComponent::OnComboWindowOpened()
{
	InputWindowStartTime = GetWorld()->GetTimeSeconds();
	CurrentComboState = EComboState::WaitingForInput;
	SetPlayerState(EPlayerState::Attack);
}

void UComboBufferComponent::OnComboWindowClosed()
{
	InputWindowEndTime = GetWorld()->GetTimeSeconds();
	
	if (BufferedInput.IsSet())
	{
		UE_LOG(LogTemp, Warning, TEXT(">>> Do the next combo..."));
		QueueChainedCombo(BufferedInput.GetValue());
		BufferedInput.Reset();
	}
}

void UComboBufferComponent::OnResetCombo()
{
	UE_LOG(LogTemp, Warning, TEXT(">>> Reset combo..."));
	bComboInProgress = false;
	CurrentComboState = EComboState::Idle;
	SetPlayerState(EPlayerState::None);
	BufferedInput.Reset();
}

int8 UComboBufferComponent::GetInputDirection() const
{
	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetOwner()))
	{
		FVector Vec = PlayerCharacter->GetLastMovementInputVector();
		return (Vec.X > 0.1f) ? 1 : (Vec.X < -0.1f ? -1 : 0);
	}
	return 0;
}

bool UComboBufferComponent::IsInAir() const
{
	if (auto* Movement = GetMovementComponent())
	{
		return Movement->IsFalling() || Movement->IsFlying();
	}
	return false;
}

UCharacterMovementComponent* UComboBufferComponent::GetMovementComponent() const
{
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		return Character->GetCharacterMovement();
	}
	return nullptr;
}

EPlayerState UComboBufferComponent::GetPlayerCurrentState() const
{
	if (APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwner()))
	{
		return Character->GetCurrentState();
	}
	return EPlayerState::None;
}

EComboTag UComboBufferComponent::GetCurrentComboTag() const
{
	return IsInAir() ? EComboTag::Air : EComboTag::Ground;
}

void UComboBufferComponent::SetMovementMode(EMovementMode Mode)
{
	if (auto* Movement = GetMovementComponent())
	{
		Movement->SetMovementMode(Mode);
	}
}

void UComboBufferComponent::SetPlayerState(EPlayerState NewPlayerState)
{
	if (APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwner()))
	{
		Character->SetCurrentState(NewPlayerState);
	}
}

void UComboBufferComponent::StartAirLaunchAttack()
{
	if (APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwner()))
	{
		if (Character->GetCurrentState() == EPlayerState::None && !IsInAir())
		{
			PlayAnimMontage(CurrentWeapon->WeaponProfile.ActionsData.AirLaunchAttack, 1.0f);
		}
	}
}
