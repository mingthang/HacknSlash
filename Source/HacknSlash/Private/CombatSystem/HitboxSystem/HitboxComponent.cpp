#include "CombatSystem/HitboxSystem/HitboxComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UHitboxComponent::UHitboxComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UHitboxComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UHitboxComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UHitboxComponent::InitMesh_Implementation(UPrimitiveComponent* InMesh)
{
	if (IsValid(InMesh))
	{
		Mesh = InMesh;
	}
	
	USkeletalMeshComponent* PlayerSK = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
	if (PlayerSK)
		HandRTransform = PlayerSK->GetBoneTransform("hand_r");
}

void UHitboxComponent::BeginAttackTrace_Implementation()
{
	// Here I create a snapshot for the attack animation is playing
	USkeletalMeshComponent* PlayerSK = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
	if (!PlayerSK)
	{
		UE_LOG(LogTemp, Error, TEXT("UHitboxComponent::BeginAttackTrace - PlayerSK is null"));
		return;
	}

	UAnimInstance* AnimInstance = PlayerSK->GetAnimInstance();
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("UHitboxComponent::BeginAttackTrace - AnimInstance is null"));
		return;
	}
	
	CurrentMontage = AnimInstance->GetCurrentActiveMontage();
	if (!CurrentMontage)
	{
		UE_LOG(LogTemp, Error, TEXT("UHitboxComponent::BeginAttackTrace - CurrentMontage is null"));
		return;
	}

	//MontageTime = AnimInstance->Montage_GetPosition(CurrentMontage);
}

void UHitboxComponent::UpdateMeleeAttackTrace_Implementation(EWeaponState InWeaponState, ETraceType InTraceType, const TArray<FName>& InDamageSockets, float InTraceSize, FVector InTraceOffset, float DeltaTime)
{
	AccumulatedTime += DeltaTime;

	while (AccumulatedTime >= FixedTimeStep)
	{
		TraceOffset = InTraceOffset;
		HandleUpdateMeleeAttackTrace(InWeaponState, InTraceType, InDamageSockets, InTraceSize);
		AccumulatedTime -= FixedTimeStep;
	}
}

void UHitboxComponent::HandleUpdateMeleeAttackTrace(EWeaponState InWeaponState, ETraceType InTraceType, const TArray<FName>& InDamageSockets, float InTraceSize)
{
	// Here using USceneComponent for future generalized use
	USceneComponent* SelectedMesh = (InWeaponState == EWeaponState::Unarmed)
	? Cast<USceneComponent>(GetOwner()->GetComponentByClass(USkeletalMeshComponent::StaticClass()))
	: Mesh;
	// Here assume for unarmed is ["hand_r"]
	TArray<FName> DamageSockets = (InWeaponState == EWeaponState::Unarmed) ? InDamageSockets : Mesh->GetAllSocketNames();
	
	if (!SelectedMesh || DamageSockets.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No valid mesh or sockets for tracing"));
		return;
	}

	if (!CurrentMontage)
    {
        UE_LOG(LogTemp, Warning, TEXT("CurrentMontage is null during attack trace"));
        return;
    }

	// Trace types
	switch (InTraceType)
	{
	case ETraceType::LineTrace:
		for (int32 i = 0; i < DamageSockets.Num(); i++)
		{
			//const FVector CurrentSocketLocation = SelectedMesh->GetSocketLocation(DamageSockets[i]);
			USkeletalMeshComponent* PlayerSK = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
			FTransform SocketTransform = GetWeaponSocketTransform(CurrentMontage, MontageTime, DamageSockets[i], PlayerSK);
			FVector CurrentSocketLocation = SocketTransform.GetLocation();
			
			FVector Start = PreviousStartLocations.IsValidIndex(i) ? PreviousStartLocations[i] : CurrentSocketLocation;
			FVector End = CurrentSocketLocation;

			HandleSingleTrace(Start, End);

			CurrentStart = Start;
			CurrentEnd = End;

			FVector NextStart = End + TraceOffset;
			if (PreviousStartLocations.IsValidIndex(i))
				PreviousStartLocations[i] = NextStart;
			else
				PreviousStartLocations.Add(NextStart);
	
			if (PreviousEndLocations.IsValidIndex(i))
				PreviousEndLocations[i] = End;
			else
				PreviousEndLocations.Add(End);
		}
		break;
	case ETraceType::SphereTrace:
		
		break;

	default:
		break;
	}
	
	MontageTime += FixedTimeStep;
}

void UHitboxComponent::HandleSingleTrace(const FVector& Start, const FVector& End)
{
	TArray<FHitResult> Hits;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	bool bHit = GetWorld()->LineTraceMultiByChannel(Hits, Start, End, ECC_GameTraceChannel1, Params);

	if (bDebugMode)
		DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1.0f, 0, 1.0f);

	if (bHit)
	{
		for (const FHitResult& Hit : Hits)
		{
			if (bDebugMode)
				DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 8.0f, 12, FColor::Green, false, 1.5f);

			if (!HitActors.Contains(Hit.GetActor()))
			{
				HitActors.AddUnique(Hit.GetActor());
				OnHitResult.Broadcast(Hit);
			}
		}
	}
}

void UHitboxComponent::EndAttackTrace_Implementation()
{
	PreviousStartLocations.Empty();
	HitActors.Empty();
	PreviousStart = FVector::Zero();
	PreviousEnd = FVector::Zero();
	AccumulatedTime = 0.0f;
}

FTransform UHitboxComponent::GetWeaponSocketTransform(UAnimMontage* InMontage, float InMontageTime, FName SocketName, USkeletalMeshComponent* InSkeletalMeshComp)
{
	// To deal with CCD, low fps leading to incorrect trace
	// Here I read raw sockets from animation sequence at the specific animation time
	if (!InMontage || !InSkeletalMeshComp || !Mesh)
		return FTransform::Identity;
	
	// Default track (for simplicity)
	const FAnimTrack& AnimTrack = InMontage->SlotAnimTracks[0].AnimTrack;

	for (const FAnimSegment& Segment : AnimTrack.AnimSegments)
	{
		const float StartTime = Segment.StartPos;
		const float EndTime = StartTime + Segment.AnimPlayRate * Segment.GetLength();

		if (InMontageTime >= StartTime && InMontageTime <= EndTime)
		{
			UAnimSequence* AnimSeq = Cast<UAnimSequence>(Segment.GetAnimReference().Get());
			if (!AnimSeq)
				return FTransform::Identity;

			// Montage time -> Local time of the animation segment
			const float AnimTime = (InMontageTime - StartTime) / Segment.AnimPlayRate;
			
			// Find socket at AnimTime
			// Transformation: Socket-Local -> Weapon-Local -> Bone-Local -> World
			// ===================================================================
			
			// Get the transformation of the bone
			/* !!! ANOTHER APPROACH IDK WHY IT DOESN'T WORK, KEEP HERE FOR FUTURE RESEARCH 
			int32 BoneIndex = InSkeletalMeshComp->GetBoneIndex("hand_r");
			if (BoneIndex == INDEX_NONE)
				return FTransform::Identity;
			int32 const TrackIndex = AnimSeq->GetSkeleton()->GetRawAnimationTrackIndex(BoneIndex, AnimSeq);
			AnimSeq->GetBoneTransform(BoneLocal, TrackIndex, AnimTime, false);
			UE_LOG(LogTemp, Log, TEXT("    -> TrackIndex = %d, BoneLocal (hand_r)=%s"), TrackIndex, *BoneLocal.ToString());
			*/
			FBoneContainer BoneContainer = InSkeletalMeshComp->GetAnimInstance()->GetRequiredBones();
			FCompactPose OutPose;
			OutPose.SetBoneContainer(&BoneContainer);
			FBlendedCurve OutCurve; 
			UE::Anim::FStackAttributeContainer OutAttr;
			
			// FAnimationPoseData PoseData; // We can't do this, FAnimationPoseData has deleted default constructor
			FAnimationPoseData PoseData(OutPose, OutCurve, OutAttr);
			FAnimExtractContext ExtractContext(AnimTime);
			AnimSeq->GetAnimationPose(PoseData, ExtractContext);

			const TArray<FTransform>& BoneTransforms =reinterpret_cast<const TArray<FTransform>&>(OutPose.GetBones());
			const FReferenceSkeleton& RefSkel = AnimSeq->GetSkeleton()->GetReferenceSkeleton();
			int32 BoneIndex = RefSkel.FindBoneIndex(FName("hand_r"));
			FTransform BoneLocalTransform = BoneTransforms[BoneIndex];
			FTransform BoneWorldTransform = BoneLocalTransform;
			while (BoneIndex != INDEX_NONE)
			{
				int32 ParentBoneIndex =	RefSkel.GetParentIndex(BoneIndex);
				if (ParentBoneIndex == INDEX_NONE)
					break;
				FTransform ParentBoneTransform = BoneTransforms[ParentBoneIndex];
				BoneWorldTransform = BoneWorldTransform * ParentBoneTransform;

				BoneIndex = ParentBoneIndex;
			}
			//BoneWorldTransform.SetLocation(BoneWorldTransform.GetLocation() + InSkeletalMeshComp->GetComponentTransform().GetLocation());
			//BoneWorldTransform =  BoneWorldTransform * InSkeletalMeshComp->GetComponentTransform();
			BoneWorldTransform =  BoneWorldTransform * InSkeletalMeshComp->GetComponentToWorld();
			int32 RootIndex = RefSkel.FindBoneIndex(FName("root"));
			if (RootIndex == INDEX_NONE)
				return FTransform::Identity;

			// Get WeaponSocket transform (SkeletalMesh local space)
			FTransform WeaponSocketLocal = InSkeletalMeshComp->GetSocketTransform(FName("WeaponSocket"), RTS_ParentBoneSpace);

			// Get Sockets (Damage Points) transform (WeaponMesh local space)
			FTransform DamageSocketLocal = Mesh->GetSocketTransform(SocketName, RTS_Component); // Assumption: Mesh is Weapon's Static Mesh
			FTransform DamageSocketWorld = DamageSocketLocal * WeaponSocketLocal * BoneWorldTransform;

			// DrawDebugSphere(GetWorld(), BoneWorldTransform.GetLocation(),5.0f, 8, FColor::Green, false, 2.0f);
			// DrawDebugSphere(GetWorld(), DamageSocketWorld.GetLocation(), 5.0f, 8, FColor::Red, false, 2.0f);

			return DamageSocketWorld;
		}
	}

	return FTransform::Identity;
}

void UHitboxComponent::SetMontageTime(float InMontageTime)
{
	MontageTime = InMontageTime;
}


