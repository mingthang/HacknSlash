#pragma once

#include "EComboTag.generated.h"

UENUM(BlueprintType)
enum class EComboTag : uint8
{
	Ground     UMETA(DisplayName = "Ground"),
	Air         UMETA(DisplayName = "Air")
};
