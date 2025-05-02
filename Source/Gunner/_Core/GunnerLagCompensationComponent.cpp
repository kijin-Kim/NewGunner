// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerLagCompensationComponent.h"

#include "GunnerActionComponent.h"
#include "GameFramework/Character.h"
#include "RewoundSnapshotAnimInstance.h"
#include "GameFramework/PlayerState.h"
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
		NewPoseSnapshot.TransformSnapshot = CharacterOwner->GetMesh()->GetComponentTransform();
		CharacterOwner->GetMesh()->SnapshotPose(NewPoseSnapshot.PoseSnapshot);

		if (NewPoseSnapshot.IsValid())
		{
			PoseSnapshots.AddFront(NewPoseSnapshot);
		}
	}
}

void UGunnerLagCompensationComponent::AuthBeginRewind(float TargetTimeStamp, FGunnerDebugHitConfirmInfo* OutDebugHitConfirmInfoPtr)
{
	if (!CharacterOwner->HasAuthority())
	{
		GR_LOG_SUB(CharacterOwner, LogGunner, Error, TEXT("권한 없는 함수 호출"));
		return;
	}


	if (PoseSnapshots.IsEmpty())
	{
		return;
	}
	

	const double EarliestAllowedTime = GetWorld()->GetTimeSeconds() - MaxRewindTime;
	TargetTimeStamp = FMath::Clamp(TargetTimeStamp, EarliestAllowedTime, GetWorld()->GetTimeSeconds());

	FMyPoseSnapshot NearestFutureSnapshot;
	FMyPoseSnapshot NearestPastSnapshot;

	NearestFutureSnapshot = PoseSnapshots.First();
	NearestPastSnapshot = NearestFutureSnapshot;
	bool bFound = false;
	for (int i = 1; i < PoseSnapshots.Num(); ++i)
	{
		if (PoseSnapshots[i].Time <= TargetTimeStamp)
		{
			bFound = true;
			NearestPastSnapshot = PoseSnapshots[i];
			break;
		}
		NearestFutureSnapshot = PoseSnapshots[i];
	}

	AuthSpawnDummyMesh(NearestFutureSnapshot, NearestPastSnapshot, TargetTimeStamp);

	if (OutDebugHitConfirmInfoPtr)
	{
		OutDebugHitConfirmInfoPtr->bRewindSnapshotFound = bFound;
		OutDebugHitConfirmInfoPtr->ServerRewindTimeStamp = TargetTimeStamp;
		OutDebugHitConfirmInfoPtr->CollectDebugHitBoxInfo(DummyMeshComponent);
	}
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
	if (!CharacterOwner->HasAuthority())
	{
		GR_LOG_SUB(CharacterOwner, LogGunner, Error, TEXT("권한 없는 함수 호출"));
	}

	const double Range = NearestFutureSnapshot.Time - NearestPastSnapshot.Time;
	const double t = TargetTime - NearestPastSnapshot.Time;

	const double Fraction = Range > KINDA_SMALL_NUMBER ? FMath::Clamp(t / Range, 0.0f, 1.0f) : 1.0f;

	FTransform NearFutuerTransform = NearestFutureSnapshot.TransformSnapshot;
	FTransform NearPastTransform = NearestPastSnapshot.TransformSnapshot;
	FVector NewLocation = FMath::Lerp(NearPastTransform.GetLocation(), NearFutuerTransform.GetLocation(), Fraction);
	FQuat NewRotation = FQuat::Slerp(NearPastTransform.GetRotation(), NearFutuerTransform.GetRotation(), Fraction);
	FTransform NewTransform{NewRotation, NewLocation};

	DummyMeshComponent = Cast<USkeletalMeshComponent>(CharacterOwner->AddComponentByClass(USkeletalMeshComponent::StaticClass(), false, FTransform::Identity, false));
	DummyMeshComponent->RegisterComponent();
	DummyMeshComponent->SetCollisionResponseToChannels(CharacterOwner->GetMesh()->GetCollisionResponseToChannels());
	DummyMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DummyMeshComponent->SetWorldTransform(NewTransform);
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
