#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "EWeaponState.h"
#include "ComboSystem/ComboStructs.h"
#include "WeaponStructs.generated.h"

class UStaticMeshComponent;

USTRUCT(BlueprintType)
struct FWeaponData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWeaponState WeaponState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* WeaponMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WeaponSocket = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName HolsterSocket;
};

USTRUCT(BlueprintType)
struct FWeaponProfile : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FWeaponData WeaponData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FComboSet ComboSet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCombatActionsData ActionsData;	
};
