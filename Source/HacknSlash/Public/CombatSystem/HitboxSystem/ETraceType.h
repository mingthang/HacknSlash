#pragma once

#include "ETraceType.generated.h"

UENUM(BlueprintType)
enum class ETraceType : uint8
{
	LineTrace UMETA(DisplayName = "LineTrace"),
	SphereTrace UMETA(DisplayName = "SphereTrace")
};
