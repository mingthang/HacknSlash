#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CombatSystem/TargetSystem/TargetSystemInterface.h"
#include "Enemy.generated.h"

class UHitboxComponent;
class UHitReactionComponent;
class UWidgetComponent;

UCLASS()
class HACKNSLASH_API AEnemy : public ACharacter, public ITargetSystemInterface
{
	GENERATED_BODY()

public:
	AEnemy();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// Target System Interface Implementation
	virtual UWidgetComponent* GetWidgetTargetComponent_Implementation() override;

protected:
	UFUNCTION()
	void OnDeath();	

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UHitboxComponent* HitboxComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UHitReactionComponent* HitReactionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* WidgetTargetComponent;
};
