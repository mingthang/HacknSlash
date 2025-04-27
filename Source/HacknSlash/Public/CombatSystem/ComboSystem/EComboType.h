#pragma once

#include "EComboType.generated.h"

UENUM(BlueprintType)
enum class EComboType : uint8
{
	BufferedCombo UMETA(DisplayName = "BufferedCombo"),
	ChainedCombo UMETA(DisplayName = "ChainedCombo")
};