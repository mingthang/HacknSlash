#include "CombatSystem/Weapon.h"
#include "Components/StaticMeshComponent.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetCollisionProfileName(TEXT("NoCollision"));
	WeaponMesh->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	RootComponent = WeaponMesh;
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

}

void AWeapon::InitWeapon(const FName& InName, const FWeaponProfile& InProfile)
{
	WeaponName = InName;
	WeaponProfile = InProfile;
	
	if (WeaponProfile.WeaponData.WeaponMesh)
	{
		WeaponMesh->SetStaticMesh(WeaponProfile.WeaponData.WeaponMesh);
	}
}


void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

