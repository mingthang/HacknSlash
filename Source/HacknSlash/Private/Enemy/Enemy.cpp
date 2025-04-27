#include "Enemy/Enemy.h"
#include "Components/CapsuleComponent.h"
#include "CombatSystem/HitboxSystem/HitboxComponent.h"
#include "CombatSystem/HitReactionSystem/HitReactionComponent.h"


AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Components
	HitboxComponent =CreateDefaultSubobject<UHitboxComponent>(TEXT("HitboxComponent"));
	HitReactionComponent = CreateDefaultSubobject<UHitReactionComponent>(TEXT("HitReactionComponent"));
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

