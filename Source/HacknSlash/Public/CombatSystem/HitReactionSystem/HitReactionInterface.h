#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HitReactionInterface.generated.h"

UINTERFACE()
class UHitReactionInterface : public UInterface
{
	GENERATED_BODY()
};

class IHitReactionInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetHitReaction(bool bIsAirLaunchAttack, float Damage, bool bHitPushBack, float PushBackDistance, USoundBase* HitSFX, UNiagaraSystem* HitParticleFX, TSubclassOf<UCameraShakeBase> CameraShake, float CameraShakeScale);
};
