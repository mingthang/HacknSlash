#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatSystem/EWeaponState.h"
#include "CombatSystem/HitboxSystem/ETraceType.h"
#include "HitboxInterface.generated.h"

UINTERFACE()
class UHitboxInterface : public UInterface
{
	GENERATED_BODY()
};

class IHitboxInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void InitMesh(UPrimitiveComponent* InMesh);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void BeginAttackTrace();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void UpdateMeleeAttackTrace(EWeaponState InWeaponState, ETraceType InTraceType, const TArray<FName>& InDamageSockets, float InTraceSize, float DeltaTime);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void EndAttackTrace();
};
