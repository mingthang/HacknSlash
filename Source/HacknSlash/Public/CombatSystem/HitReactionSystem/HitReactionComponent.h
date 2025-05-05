#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HitReactionInterface.h"
#include "HitReactionComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACKNSLASH_API UHitReactionComponent : public UActorComponent, public IHitReactionInterface
{
	GENERATED_BODY()

public:	
	UHitReactionComponent();

	// HitReaction Interface Implementation
	virtual void SetHitReaction_Implementation(bool bIsAirLaunchAttack, float Damage, bool bHitPushBack, float PushBackDistance, USoundBase* HitSFX, UNiagaraSystem* HitParticle, TSubclassOf<UCameraShakeBase> CameraShake, float CameraShakeScale) override;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void OnHitReaction(const float Damage, AActor* DamageCauser, const FHitResult& HitInfo, FName DamagedActorTag);

	UFUNCTION(BlueprintCallable)
	void LaunchCharacter(float LaunchVelocityZ);

	UFUNCTION(BlueprintCallable)
	float GetStamina() { return Stamina; }
	UFUNCTION(BlueprintCallable)
	void RemoveStamina(const float Amount);
	void RegenStamina();

	UFUNCTION(BlueprintCallable)
	float GetDamage() const { return Damage; }
	
	UFUNCTION(BlueprintCallable)
	bool IsAlive() const { return bIsAlive; }
	
	UFUNCTION(BlueprintCallable)
	bool IsHit() const { return bIsHit; }
	UFUNCTION(BlueprintCallable)
	void SetIsHit(bool IsHit);

	// HIT REACTIONS MONTAGES
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Hit Reaction Anim Montages")
	UAnimMontage* FrontHitReactionMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Hit Reaction Anim Montages")
	UAnimMontage* BackHitReactionMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Hit Reaction Anim Montages")
	UAnimMontage* AirHitReactionMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Hit Reaction Anim Montages")
	UAnimMontage* AirLaunchReactionMontage;

protected:
	void HitFeedBackAnim(UAnimMontage* HitMontage, const AActor* TargetActor);
	void HitFeedBackFX();

	void RagdollEvent();
	void OnDeath();
	
	void PushBack(float PushBackDistance);
	bool IsBackHit();
	void DoCameraShake();
	
	UAnimMontage* GetHitMontage();

private:
	// Stats
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats", meta = (AllowPrivateAccess = "true"))
	float MaxHealth = 1000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats", meta = (AllowPrivateAccess = "true"))
	float Health = 1000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats", meta = (AllowPrivateAccess = "true"))
	float MaxStamina = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats", meta = (AllowPrivateAccess = "true"))
	float Stamina = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats", meta = (AllowPrivateAccess = "true"))
	float StaminaRegenPerSec = 0.1f;
	FTimerHandle StaminaRegenHandle;

	// Combat
	bool bIsAirLaunchAttack = false;
	float Damage = 0.0f;
	
	bool bHitPushBack = false;
	float PushBackDistance = 20.0f;
	bool bIsPushingBack = false;
	FVector PushTargetLocation;
	float PushInterpSpeed = 7.0f;
	
	UPROPERTY() USoundBase* HitSFX = nullptr;
	UPROPERTY() UNiagaraSystem* HitParticleFX = nullptr;
	UPROPERTY() TSubclassOf<UCameraShakeBase> CameraShake;
	float CameraShakeScale = 0.0f;

	FHitResult HitInfo;

	bool bIsAlive = true;
	bool bIsHit = false;
	UPROPERTY() AActor* DamageCauser = nullptr;
};
