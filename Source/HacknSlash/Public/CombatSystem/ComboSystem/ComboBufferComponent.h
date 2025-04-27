#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatSystem/Weapon.h"
#include "CombatSystem/ComboSystem/EComboInput.h"
#include "Player/EPlayerState.h"
#include "ComboBufferComponent.generated.h"

class USkeletalMeshComponent;
class UAnimMontage;
class APlayerCharacter;
class UCharacterMovementComponent;

UENUM(BlueprintType)
enum class EComboState : uint8
{
	Idle,
	Executing,
	WaitingForInput,
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HACKNSLASH_API UComboBufferComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UComboBufferComponent();

	// Init
	UFUNCTION(BlueprintCallable, Category = "Weapon|Combo")
	void InitWeaponStates(USkeletalMeshComponent* SkeletalMeshComponent);

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	float PlayAnimMontage(UAnimMontage* Montage, float PlayRate);

	// Input Handlers
	void OnComboButtonDown(EComboInput Input);
	void OnComboButtonUp(EComboInput Input);
	void SetComboInput(bool IsPressed, EComboInput ComboInput);
	
	void StartAirLaunchAttack();
	
protected:
	void EquipWeapon(int32 WeaponIndex);

	// Combo Logic
	void StartNewComboChain(EComboInput Input);
	void QueueChainedCombo(EComboInput Input);
	void BufferComboInput(EComboInput Input);

	// AnimNotify hooks 
	UFUNCTION(BlueprintCallable)
	void OnComboWindowOpened();
	UFUNCTION(BlueprintCallable)
	void OnComboWindowClosed();
	UFUNCTION(BlueprintCallable)
	void OnResetCombo();

	// Helpers
	bool IsInAir() const;
	UCharacterMovementComponent* GetMovementComponent() const;
	EPlayerState GetPlayerCurrentState() const;
	EComboTag GetCurrentComboTag() const;
	int8 GetInputDirection() const;
	float GetCurrentHoldDuration() const { return LastInputTimeUp - CurrentInputStartTime; }
    void SetMovementMode(EMovementMode Mode);
	void SetPlayerState(EPlayerState NewPlayerState);

private:
	UPROPERTY()
	USkeletalMeshComponent* SkeletalMeshComponent = nullptr;

	// Weapon
	UPROPERTY() TArray<FName> CurrentWeapons = { FName("GreatSword"), FName("Unarmed") };
	UPROPERTY() TArray<AWeapon*> SpawnedWeapons;
	UPROPERTY() AWeapon* CurrentWeapon = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon", meta = (AllowPrivateAccess = "true")) int32 CurrentWeaponIndex = INDEX_NONE;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon", meta = (AllowPrivateAccess = "true")) UDataTable* WeaponProfileTable;

	// Combo State
	EComboState CurrentComboState = EComboState::Idle;
	TArray<EComboInput> BufferedInputs;	 // for buffered combo
	TOptional<EComboInput> BufferedInput;
    int32 CurrentComboIndex = 0;
    EComboTag CurrentComboTag = EComboTag::Ground;
    bool bComboInProgress = false;
	
	// Input Timing
	float Delta = 0.0f;
	float LastInputTime = 0.f;
	float LastInputTimeUp = 0.f;
	float CurrentInputStartTime = 0.f;
	int8 LastInputDirection = 0;
	int32 LastBranchIndex = -1;
	float InputWindowStartTime = 0.0f;
	float InputWindowEndTime = 0.0f;

	// Combo Window
	FTimerHandle ComboWindowTimer;

	// Debug
	bool dDebugMode = false;
};