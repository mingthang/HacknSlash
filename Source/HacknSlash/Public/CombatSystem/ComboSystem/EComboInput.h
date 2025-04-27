#pragma once

#include "EComboInput.generated.h"

UENUM(BlueprintType)
enum class EComboInput : uint8
{
	None UMETA(DisplayName = "None"),
	LightAttack UMETA(DisplayName = "LightAttack"),
	HeavyAttack UMETA(DisplayName = "HeavyAttack")
};
