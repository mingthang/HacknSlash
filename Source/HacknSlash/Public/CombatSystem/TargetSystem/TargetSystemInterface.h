#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TargetSystemInterface.generated.h"

UINTERFACE(Blueprintable)
class UTargetSystemInterface : public UInterface
{
	GENERATED_BODY()
};

class ITargetSystemInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UWidgetComponent* GetWidgetTargetComponent();
};