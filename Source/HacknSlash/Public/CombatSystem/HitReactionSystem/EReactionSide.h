#pragma once

#include "EReactionSide.generated.h"

UENUM(BlueprintType)
enum class EReactionSide : uint8
{
	LineTrace UMETA(DisplayName = "LineTrace"),
	SphereTrace UMETA(DisplayName = "SphereTrace")
};
