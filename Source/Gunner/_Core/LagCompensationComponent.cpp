// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"

#include "HitBoxActorInterface.h"
#include "Components/CapsuleComponent.h"
#include "Containers/RingBuffer.h"
#include "GameFramework/GameModeBase.h"
#include "Gunner/LagCompensationDummyActor.h"
#include "Gunner/LagCompensationHitBoxCapsuleComponent.h"
#include "Gunner/Character/GunnerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/PhysicsAsset.h"


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
}

void ULagCompensationComponent::DrawHitBoxes(const TArray<FHitBox>& HitBoxes, FColor Color, bool bPersistentLines, float Time)
{
	for (const auto& [Transform, HalfHeight, Radius, BoneName] : HitBoxes)
	{
		DrawDebugCapsule(GetWorld(), Transform.GetLocation(), HalfHeight, Radius, Transform.GetRotation(), Color, bPersistentLines, Time);
	}
}
