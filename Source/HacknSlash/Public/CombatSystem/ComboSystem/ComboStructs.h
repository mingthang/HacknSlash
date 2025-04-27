#pragma once

#include "CoreMinimal.h"
#include "EComboType.h"
#include "EComboTag.h"
#include "EComboInput.h"
#include "Animation/AnimMontage.h"
#include "ComboStructs.generated.h"

USTRUCT(BlueprintType)

struct FComboBranch
{
	GENERATED_BODY()

	// Thời gian tính từ lần nhấn trước đến lần nhấn này (delta): mash vs delay
	UPROPERTY(EditAnywhere) float MinDeltaTime = 0.f;
	UPROPERTY(EditAnywhere) float MaxDeltaTime = 1.f;

	// Hướng input: -1 (back), 0 (neutral), +1 (forward)
	UPROPERTY(EditAnywhere) int8  Direction = 0;

	// Hold hay tap: nếu tap thì Duration <= TapThreshold, nếu hold thì >= HoldThreshold
	UPROPERTY(EditAnywhere) bool bRequireHold = false;
	UPROPERTY(EditAnywhere) float HoldThreshold = 0.3f;

	// Combo input (light/heavy)
	UPROPERTY(EditAnywhere) EComboInput ComboInput;

	UPROPERTY(EditAnywhere)
	int32 RequiresPreviousBranch = -1; 

	// Montage 
	UPROPERTY(EditAnywhere) UAnimMontage* Montage = nullptr;
};

USTRUCT(BlueprintType)
struct FChainedComboStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) TArray<FComboBranch> Branches;
};

USTRUCT(BlueprintType)
struct FChainedCombo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) TArray<FChainedComboStep> Steps;
};

USTRUCT(BlueprintType)
struct FChainedComboData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EComboTag ComboTag;

	// Chained Combo
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FChainedCombo ChainedCombo;	
};

USTRUCT(BlueprintType)
struct FComboSet
{
	GENERATED_BODY()

	// TODO: Buffer Combos?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EComboType ComboType = EComboType::ChainedCombo;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FChainedComboData> ChainedCombos;
};

USTRUCT(BlueprintType)
struct FCombatActionsData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* EquipMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* HolsterMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* DodgeMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* AirLaunchAttack = nullptr;
};