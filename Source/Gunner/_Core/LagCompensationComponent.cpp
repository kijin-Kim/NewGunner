// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"

#include "HitBoxActorInterface.h"
#include "Components/CapsuleComponent.h"
#include "Containers/RingBuffer.h"
#include "GameFramework/GameModeBase.h"
#include "Gunner/LagCompensationDummyActor.h"
#include "Gunner/LagCompensationHitBoxCapsuleComponent.h"
#include "Gunner/RewoundSnapshotAnimInstance.h"
#include "Gunner/Character/GunnerCharacter.h"
#include "Kismet/GameplayStatics.h"


ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
}

void ULagCompensationComponent::InitializeComponent()
{
	Super::InitializeComponent();
	check(GetOwner()->IsA(AGameModeBase::StaticClass()));
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	RecordHitBoxHistories();
}

void ULagCompensationComponent::BeginRewind(float TimeStamp, const TArray<AActor*>& RewindTargets)
{
	double SingleTripTimeFromClient = GetWorld()->GetTimeSeconds() - TimeStamp;
	double AdjustedClientTimeStamp = TimeStamp - SingleTripTimeFromClient; // 클라이언트가 Remote Client에 대해 서버보다 과거 위치를 보는 것을 반영합니다.
	double TargetTime = FMath::Min(GetWorld()->GetTimeSeconds(), AdjustedClientTimeStamp);


	FHitBoxHistory NearestFutureHistory;
	FHitBoxHistory NearestPastHistory;

	NearestFutureHistory = HitBoxHistories.First();
	for (int i = 1; i < HitBoxHistories.Num(); ++i)
	{
		if (HitBoxHistories[i].Time <= TargetTime)
		{
			NearestPastHistory = HitBoxHistories[i];
			break;
		}
		NearestFutureHistory = HitBoxHistories[i];
	}

	if (NearestPastHistory.HitBoxes.IsEmpty())
	{
		return;
	}


	FHitBoxHistory RewoundHistory;
	for (AActor* GunnerActor : RewindTargets)
	{
		RewoundHistory.HitBoxes.FindOrAdd(GunnerActor) = NearestFutureHistory.HitBoxes[GunnerActor];

		double Range = NearestFutureHistory.Time - NearestPastHistory.Time;
		double t = TargetTime - NearestPastHistory.Time;

		for (int i = 0; i < NearestFutureHistory.HitBoxes[GunnerActor].Num(); ++i)
		{
			FVector Location1 = NearestPastHistory.HitBoxes[GunnerActor][i].Transform.GetLocation();
			FVector Location2 = NearestFutureHistory.HitBoxes[GunnerActor][i].Transform.GetLocation();
			double Fraction = FMath::Clamp(t / Range, 0.0f, 1.0f);
			FVector NewLocation = FMath::VInterpTo(Location1, Location2, 1.0f, Fraction);

			FQuat Rotation1 = NearestPastHistory.HitBoxes[GunnerActor][i].Transform.GetRotation();
			FQuat Rotation2 = NearestFutureHistory.HitBoxes[GunnerActor][i].Transform.GetRotation();
			FQuat NewRotation = FMath::QInterpTo(Rotation1, Rotation2, 1.0f, Fraction);
			FTransform NewTransform{NewRotation, NewLocation};

			RewoundHistory.HitBoxes[GunnerActor][i].Transform = NewTransform;
		}
	}

	SpawnDummies(RewoundHistory, RewindTargets);
}


void ULagCompensationComponent::BeginRewind2(float TimeStamp, const TArray<AActor*>& RewindTargets)
{
	double SingleTripTimeFromClient = GetWorld()->GetTimeSeconds() - TimeStamp;
	double AdjustedClientTimeStamp = TimeStamp - SingleTripTimeFromClient;
	double TargetTime = FMath::Min(GetWorld()->GetTimeSeconds(), AdjustedClientTimeStamp);

	FPoseSnapshotHistory NearestFutureHistory;
	FPoseSnapshotHistory NearestPastHistory;

	NearestFutureHistory = PoseSnapshots.First();
	NearestPastHistory = NearestFutureHistory;
	for (int i = 1; i < PoseSnapshots.Num(); ++i)
	{
		if (PoseSnapshots[i].Time <= TargetTime)
		{
			NearestPastHistory = PoseSnapshots[i];
			break;
		}
		NearestFutureHistory = PoseSnapshots[i];
	}
	SpawnDummies2(NearestFutureHistory, NearestPastHistory, RewindTargets, TargetTime);
}


void ULagCompensationComponent::SpawnDummies(const FHitBoxHistory& RewoundHistory, const TArray<AActor*>& RewindTargets)
{
	for (AActor* GunnerActor : RewindTargets)
	{
		AActor* RewindedDummy = GetWorld()->SpawnActorDeferred<AActor>(ALagCompensationDummyActor::StaticClass(), FTransform::Identity, GunnerActor);
		RewindedDummies.Add(RewindedDummy);
		RewindedDummy->AddComponentByClass(USceneComponent::StaticClass(), false, FTransform::Identity, false);
		for (const FHitBox& HitBox : RewoundHistory.HitBoxes[GunnerActor])
		{
			ULagCompensationHitBoxCapsuleComponent* CapsuleComponent = Cast<ULagCompensationHitBoxCapsuleComponent>(RewindedDummy->AddComponentByClass(ULagCompensationHitBoxCapsuleComponent::StaticClass(), false, HitBox.Transform, false));
			CapsuleComponent->SetBoneName(HitBox.BoneName);
			CapsuleComponent->SetCapsuleRadius(HitBox.Radius);
			CapsuleComponent->SetCapsuleHalfHeight(HitBox.HalfHeight);
		}
		RewindedDummy->FinishSpawning(GunnerActor->GetActorTransform());
	}
}

void ULagCompensationComponent::SpawnDummies2(const FPoseSnapshotHistory& NearestFutureHistory, const FPoseSnapshotHistory& NearestPastHistory, const TArray<AActor*>& RewindTargets, float TargetTime)
{
	const double Range = NearestFutureHistory.Time - NearestPastHistory.Time;
	const double t = TargetTime - NearestPastHistory.Time;
	const double Fraction = FMath::Clamp(t / Range, 0.0f, 1.0f);


	for (AActor* RewindTarget : RewindTargets)
	{
		if (ACharacter* RewindTargetCharacter = Cast<ACharacter>(RewindTarget))
		{
			FTransform NearFutuerTransform = NearestFutureHistory.TransformSnapShots[RewindTarget];
			FTransform NearPastTransform = NearestPastHistory.TransformSnapShots[RewindTarget];
			FVector NewLocation = FMath::VInterpTo(NearPastTransform.GetLocation(), NearFutuerTransform.GetLocation(), 1.0f, Fraction);
			FQuat NewRotation = FMath::QInterpTo(NearPastTransform.GetRotation(), NearFutuerTransform.GetRotation(), 1.0f, Fraction);
			FTransform NewTransform{NewRotation, NewLocation};

			USkeletalMeshComponent* DummySkeletalMeshComponent = Cast<USkeletalMeshComponent>(RewindTargetCharacter->AddComponentByClass(USkeletalMeshComponent::StaticClass(), false, FTransform::Identity, false));
			DummySkeletalMeshComponent->RegisterComponent();
			RewindedDummies2.Add(DummySkeletalMeshComponent);

			DummySkeletalMeshComponent->SetCollisionResponseToChannels(RewindTargetCharacter->GetMesh()->GetCollisionResponseToChannels());
			DummySkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			DummySkeletalMeshComponent->SetWorldTransform(NewTransform * RewindTargetCharacter->GetMesh()->GetRelativeTransform());
			DummySkeletalMeshComponent->SetSkeletalMesh(RewindTargetCharacter->GetMesh()->GetSkeletalMeshAsset());
			DummySkeletalMeshComponent->SetPhysicsAsset(RewindTargetCharacter->GetMesh()->GetPhysicsAsset());
			DummySkeletalMeshComponent->SetAnimInstanceClass(DummyAnimInstanceClass);
			URewoundSnapshotAnimInstance* RewoundSnapshotAnimInstance = Cast<URewoundSnapshotAnimInstance>(DummySkeletalMeshComponent->GetAnimInstance());
			RewoundSnapshotAnimInstance->SetBlendAlpha(Fraction);

			FPoseSnapshot& NearestFutureSnapshot = RewoundSnapshotAnimInstance->AddPoseSnapshot(FName("RewindSnapshot_NearestFuture"));
			NearestFutureSnapshot.LocalTransforms = NearestFutureHistory.PoseSnapShots[RewindTarget].LocalTransforms;
			NearestFutureSnapshot.BoneNames = NearestFutureHistory.PoseSnapShots[RewindTarget].BoneNames;
			NearestFutureSnapshot.SkeletalMeshName = NearestFutureHistory.PoseSnapShots[RewindTarget].SkeletalMeshName;

			FPoseSnapshot& NearestPastSnapshot = RewoundSnapshotAnimInstance->AddPoseSnapshot(FName("RewindSnapshot_NearestPast"));
			NearestPastSnapshot.LocalTransforms = NearestPastHistory.PoseSnapShots[RewindTarget].LocalTransforms;
			NearestPastSnapshot.BoneNames = NearestPastHistory.PoseSnapShots[RewindTarget].BoneNames;
			NearestPastSnapshot.SkeletalMeshName = NearestPastHistory.PoseSnapShots[RewindTarget].SkeletalMeshName;

			DummySkeletalMeshComponent->TickAnimation(0.0f, false);
			DummySkeletalMeshComponent->RefreshBoneTransforms();
			DummySkeletalMeshComponent->RefreshFollowerComponents();
			DummySkeletalMeshComponent->UpdateComponentToWorld();
			DummySkeletalMeshComponent->FinalizeBoneTransform();
		}
	}
}

void ULagCompensationComponent::EndRewind()
{
	for (AActor* Dummies : RewindedDummies)
	{
		if (Dummies)
		{
			Dummies->Destroy();
		}
	}
	RewindedDummies.Empty();
}

void ULagCompensationComponent::EndRewind2()
{
	for (USkeletalMeshComponent* Dummies : RewindedDummies2)
	{
		if (Dummies)
		{
			Dummies->DestroyComponent();
		}
	}
	RewindedDummies2.Empty();
}

void ULagCompensationComponent::RecordHitBoxHistories()
{
	TArray<AActor*> HitBoxActors;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UHitBoxActorInterface::StaticClass(), HitBoxActors);
	if (HitBoxActors.IsEmpty())
	{
		return;
	}

	while (!HitBoxHistories.IsEmpty() && GetWorld()->GetTimeSeconds() - HitBoxHistories.Last().Time >= MaxRewindTime)
	{
		HitBoxHistories.Pop();
	}

	FHitBoxHistory NewHistory;
	NewHistory.Time = GetWorld()->GetTimeSeconds();
	for (AActor* HitBoxActor : HitBoxActors)
	{
		TArray<FHitBox>& HitBoxes = NewHistory.HitBoxes.FindOrAdd(HitBoxActor);
		IHitBoxActorInterface* HitBoxActorInterface = Cast<IHitBoxActorInterface>(HitBoxActor);
		check(HitBoxActorInterface);
		HitBoxes = HitBoxActorInterface->CollectAndGetHitBoxes();
	}

	HitBoxHistories.AddFront(NewHistory);


	while (!PoseSnapshots.IsEmpty() && GetWorld()->GetTimeSeconds() - PoseSnapshots.Last().Time >= MaxRewindTime)
	{
		PoseSnapshots.Pop();
	}

	FPoseSnapshotHistory NewPoseSnapshotHistory;
	NewPoseSnapshotHistory.Time = GetWorld()->GetTimeSeconds();
	for (AActor* HitBoxActor : HitBoxActors)
	{
		if (ACharacter* HitBoxCharacter = Cast<ACharacter>(HitBoxActor))
		{
			FTransform& NewTransformSnapshot = NewPoseSnapshotHistory.TransformSnapShots.FindOrAdd(HitBoxActor);
			NewTransformSnapshot = HitBoxCharacter->GetActorTransform();
			FPoseSnapshot& NewPoseSnapshot = NewPoseSnapshotHistory.PoseSnapShots.FindOrAdd(HitBoxActor);
			HitBoxCharacter->GetMesh()->SnapshotPose(NewPoseSnapshot);
		}
	}

	if (NewPoseSnapshotHistory.IsValid())
	{
		PoseSnapshots.AddFront(NewPoseSnapshotHistory);
	}
}

void ULagCompensationComponent::DrawHitBoxes(const TArray<FHitBox>& HitBoxes, FColor Color, bool bPersistentLines, float Time)
{
	for (const auto& [Transform, HalfHeight, Radius, BoneName] : HitBoxes)
	{
		DrawDebugCapsule(GetWorld(), Transform.GetLocation(), HalfHeight, Radius, Transform.GetRotation(), Color, bPersistentLines, Time);
	}
}
