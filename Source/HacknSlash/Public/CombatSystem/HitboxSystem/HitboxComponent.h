#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HitboxInterface.h"
#include "HitboxComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitResult, const FHitResult&, HitResult);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACKNSLASH_API UHitboxComponent : public UActorComponent, public IHitboxInterface
{
	GENERATED_BODY()

public:	
	UHitboxComponent();

	// IHitBoxInterface implementation
	virtual void InitMesh_Implementation(UPrimitiveComponent* InMesh) override;
	virtual void BeginAttackTrace_Implementation() override;
	virtual void UpdateMeleeAttackTrace_Implementation(EWeaponState InWeaponState, ETraceType InTraceType, const TArray<FName>& InDamageSockets, float InTraceSize, FVector InTraceOffset, float DeltaTime) override;
	virtual void EndAttackTrace_Implementation() override;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// DELEGATES
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHitResult OnHitResult;

	UFUNCTION(BlueprintCallable)
	void SetMontageTime(float InMontageTime);
	
	// DEBUG
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug")
	bool bDebugMode = false;

protected:
    void HandleUpdateMeleeAttackTrace(EWeaponState InWeaponState, ETraceType InTraceType, const TArray<FName>& InDamagePoints, float InTraceSize);
	void HandleSingleTrace(const FVector& Start, const FVector& End);

	FTransform GetWeaponSocketTransform(UAnimMontage* InMontage, float InMontageTime, FName SocketName, USkeletalMeshComponent* InSkeletalMeshComp);
	FTransform GetBoneTransform(UAnimMontage* InMontage, float InMontageTime, FName BoneName, USkeletalMeshComponent* InSkeletalMesh);

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Hitbox", meta=(AllowPrivateAccess=true))
	UPrimitiveComponent* Mesh = nullptr; // Can be any but a USceneComponent-related mesh

	UPROPERTY()
	TArray<AActor*> HitActors;
	UPROPERTY()
	TArray<FVector> PreviousStartLocations;
	UPROPERTY()
	TArray<FVector> PreviousEndLocations;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Hitbox", meta=(AllowPrivateAccess=true))
	FVector TraceOffset = FVector(0.0f, 0.0f, 1.0f);

	// Fill gaps
	FVector CurrentStart = FVector::Zero();
	FVector CurrentEnd = FVector::Zero();
	FVector PreviousStart = FVector::Zero();
	FVector PreviousEnd = FVector::Zero();

	FTransform HandRTransform = FTransform::Identity;
	float AccumulatedTime = 0.0f;
	float FixedTimeStep  = 1.0f / 120.0f;
	UPROPERTY() UAnimMontage* CurrentMontage = nullptr;
	float MontageTime = 0.0f;

};