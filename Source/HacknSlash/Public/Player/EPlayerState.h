#pragma once

#include "EPlayerState.generated.h"

UENUM(BlueprintType)
enum class EPlayerState : uint8
{
	None UMETA(DisplayName = "None"),
	Attack UMETA(DisplayName = "Attack"),
	Roll UMETA(DisplayName = "Roll"),
	WeaponHandling UMETA(DisplayName = "WeaponHandling")
};