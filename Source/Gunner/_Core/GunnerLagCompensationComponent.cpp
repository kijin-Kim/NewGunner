// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerLagCompensationComponent.h"

#include "GameFramework/Character.h"
#include "RewoundSnapshotAnimInstance.h"
#include "Gunner/Gunner.h"
#include "PhysicsEngine/PhysicsAsset.h"


UGunnerLagCompensationComponent::UGunnerLagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGunnerLagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
	CharacterOwner = Cast<ACharacter>(GetOwner());
	check(CharacterOwner);
}

void UGunnerLagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (CharacterOwner->HasAuthority())
	{
		FMyPoseSnapshot NewPoseSnapshot;
		while (!PoseSnapshots.IsEmpty() && GetWorld()->GetTimeSeconds() - PoseSnapshots.Last().Time >= MaxRewindTime)
		{
			PoseSnapshots.Pop();
		}

		NewPoseSnapshot.Time = GetWorld()->GetTimeSeconds();
		NewPoseSnapshot.TransformSnapshot = CharacterOwner->GetActorTransform();
		CharacterOwner->GetMesh()->SnapshotPose(NewPoseSnapshot.PoseSnapshot);

		if (NewPoseSnapshot.IsValid())
		{
			PoseSnapshots.AddFront(NewPoseSnapshot);
		}
	}
}

bool UGunnerLagCompensationComponent::AuthBeginRewind(float TimeStamp, double& OutRewoundedTimeStamp)
{
	if (!CharacterOwner->HasAuthority())
	{
		GR_LOG_SUB(CharacterOwner, LogGunner, Error, TEXT("권한 없는 함수 호출"));
		return 0.0;
	}

	const double SingleTripTimeFromClient = GetWorld()->GetTimeSeconds() - TimeStamp;
	const double AdjustedClientTimeStamp = TimeStamp - SingleTripTimeFromClient;
	const double TargetTime = FMath::Min(GetWorld()->GetTimeSeconds(), AdjustedClientTimeStamp);

	FMyPoseSnapshot NearestFutureSnapshot;
	FMyPoseSnapshot NearestPastSnapshot;

	NearestFutureSnapshot = PoseSnapshots.First();
	NearestPastSnapshot = NearestFutureSnapshot;
	bool bFound = false;
	for (int i = 1; i < PoseSnapshots.Num(); ++i)
	{
		if (PoseSnapshots[i].Time <= TargetTime)
		{
			bFound = true;
			NearestPastSnapshot = PoseSnapshots[i];
			break;
		}
		NearestFutureSnapshot = PoseSnapshots[i];
	}
	AuthSpawnDummyMesh(NearestFutureSnapshot, NearestPastSnapshot, TargetTime);
	OutRewoundedTimeStamp = FMath::Max(NearestPastSnapshot.Time, TargetTime);
	return bFound;
}

void UGunnerLagCompensationComponent::AuthEndRewind()
{
	if (!CharacterOwner->HasAuthority())
	{
		GR_LOG_SUB(CharacterOwner, LogGunner, Error, TEXT("권한 없는 함수 호출"));
		return;
	}

	if (DummyMeshComponent)
	{
		DummyMeshComponent->DestroyComponent();
	}
}

void UGunnerLagCompensationComponent::AuthSpawnDummyMesh(const FMyPoseSnapshot& NearestFutureSnapshot, const FMyPoseSnapshot& NearestPastSnapshot, float TargetTime)
{
	if (CharacterOwner->HasAuthority())
	{
		const double Range = NearestFutureSnapshot.Time - NearestPastSnapshot.Time;
		const double t = TargetTime - NearestPastSnapshot.Time;
		const double Fraction = FMath::Clamp(t / Range, 0.0f, 1.0f);

		FTransform NearFutuerTransform = NearestFutureSnapshot.TransformSnapshot;
		FTransform NearPastTransform = NearestPastSnapshot.TransformSnapshot;
		FVector NewLocation = FMath::VInterpTo(NearPastTransform.GetLocation(), NearFutuerTransform.GetLocation(), 1.0f, Fraction);
		FQuat NewRotation = FMath::QInterpTo(NearPastTransform.GetRotation(), NearFutuerTransform.GetRotation(), 1.0f, Fraction);
		FTransform NewTransform{NewRotation, NewLocation};

		DummyMeshComponent = Cast<USkeletalMeshComponent>(CharacterOwner->AddComponentByClass(USkeletalMeshComponent::StaticClass(), false, FTransform::Identity, false));
		//DummyMeshComponent->RegisterComponent();
		DummyMeshComponent->SetCollisionResponseToChannels(CharacterOwner->GetMesh()->GetCollisionResponseToChannels());
		DummyMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		DummyMeshComponent->SetWorldTransform(NewTransform * CharacterOwner->GetMesh()->GetRelativeTransform());
		DummyMeshComponent->SetSkeletalMesh(CharacterOwner->GetMesh()->GetSkeletalMeshAsset());
		DummyMeshComponent->SetPhysicsAsset(CharacterOwner->GetMesh()->GetPhysicsAsset());
		DummyMeshComponent->SetAnimInstanceClass(PoseSnapshotAnimInstanceClass);

		URewoundSnapshotAnimInstance* RewoundSnapshotAnimInstance = Cast<URewoundSnapshotAnimInstance>(DummyMeshComponent->GetAnimInstance());
		RewoundSnapshotAnimInstance->SetBlendAlpha(Fraction);

		FPoseSnapshot& DummyNearestFutureSnapshot = RewoundSnapshotAnimInstance->AddPoseSnapshot(FName("RewindSnapshot_NearestFuture"));
		DummyNearestFutureSnapshot.LocalTransforms = NearestFutureSnapshot.PoseSnapshot.LocalTransforms;
		DummyNearestFutureSnapshot.BoneNames = NearestFutureSnapshot.PoseSnapshot.BoneNames;
		DummyNearestFutureSnapshot.SkeletalMeshName = NearestFutureSnapshot.PoseSnapshot.SkeletalMeshName;

		FPoseSnapshot& DummyNearestPastSnapshot = RewoundSnapshotAnimInstance->AddPoseSnapshot(FName("RewindSnapshot_NearestPast"));
		DummyNearestPastSnapshot.LocalTransforms = NearestPastSnapshot.PoseSnapshot.LocalTransforms;
		DummyNearestPastSnapshot.BoneNames = NearestPastSnapshot.PoseSnapshot.BoneNames;
		DummyNearestPastSnapshot.SkeletalMeshName = NearestPastSnapshot.PoseSnapshot.SkeletalMeshName;

		DummyMeshComponent->TickAnimation(0.0f, false);
		DummyMeshComponent->RefreshBoneTransforms();
		DummyMeshComponent->RefreshFollowerComponents();
		DummyMeshComponent->UpdateComponentToWorld();
		DummyMeshComponent->FinalizeBoneTransform();
	}
}
