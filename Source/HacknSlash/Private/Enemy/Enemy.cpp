#include "Enemy/Enemy.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "CombatSystem/HitboxSystem/HitboxComponent.h"
#include "CombatSystem/HitReactionSystem/HitReactionComponent.h"


AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Components
	HitboxComponent = CreateDefaultSubobject<UHitboxComponent>(TEXT("HitboxComponent"));
	HitReactionComponent = CreateDefaultSubobject<UHitReactionComponent>(TEXT("HitReactionComponent"));
	WidgetTargetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetTargetComponent"));
	WidgetTargetComponent->SetupAttachment(GetMesh(), FName("spine_03"));

	// Delegate Events
	if (HitReactionComponent)
	{
		HitReactionComponent->OnDeathEvent.AddDynamic(this, &AEnemy::OnDeath);
	}
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

void AEnemy::OnDeath()
{
	// Can do stuff like add exp to player,...
	FTimerHandle TimerHandle;
	FTimerDelegate TimerDel;
	TimerDel.BindLambda([this]()
	{
		Destroy();
	});
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, 2.0f, false);
}


UWidgetComponent* AEnemy::GetWidgetTargetComponent_Implementation()
{
	return WidgetTargetComponent;
}

