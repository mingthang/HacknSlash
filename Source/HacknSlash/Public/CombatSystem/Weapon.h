#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponStructs.h"
#include "Weapon.generated.h"

class UStaticMeshComponent;

UCLASS()
class HACKNSLASH_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();

	void InitWeapon(const FName& WeaponName, const FWeaponProfile& InProfile);

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon", meta = (ExposeOnSpawn = true))
	FName WeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	FWeaponProfile WeaponProfile;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category="Weapon")
	class UStaticMeshComponent* WeaponMesh;
};
