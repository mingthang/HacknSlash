#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TargetSystem.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACKNSLASH_API UTargetSystem : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTargetSystem();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void LockOnTarget();

	UFUNCTION(BlueprintCallable)
	void NextNearestTarget();
	
	void Reset();

	UFUNCTION(BlueprintCallable)
	void ApproachToTarget(float RequiredDistanceToFaceTarget, float RequiredDistanceToMoveToTarget, bool bSwitchToNearestEnemy, float ApproachTargetOffset, float InterpSpeed, bool bMoveToTarget);

	UFUNCTION(BlueprintCallable)
	AActor* GetLockedOnTarget() const { return LockedOnTarget; }
	UFUNCTION(BlueprintCallable)
	bool GetIsTarget() const { return bIsTarget; }
	UFUNCTION(BlueprintCallable)
	void SetLockedOnTarget(AActor* LockedOnTarget);

protected:
	bool FindTargetsInRadius(const float RadiusToFind, const TArray<AActor*>& ActorsToIgnore, ECollisionChannel ObjectTypeQuery, TArray<FHitResult>& OutHits);
	void GetNearestTarget();
	
private:
	UPROPERTY() AActor* TargetActor = nullptr;
	UPROPERTY() TArray<AActor*> TargetActors;
	bool bIsTarget = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = TargetSystem, meta = (AllowPrivateAccess = "true"))
	float FindTargetRadius = 500.0f;

	UPROPERTY() AActor* LockedOnTarget;

	FRotator LookAtTargetRotation = FRotator::ZeroRotator;
	
};
