#pragma once

#include "EWeaponState.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	Unarmed UMETA(DisplayName = "Unarmed"),
	GreatSword UMETA(DisplayName = "GreatSword")
};
