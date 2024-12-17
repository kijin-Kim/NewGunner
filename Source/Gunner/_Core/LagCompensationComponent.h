// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitBox.h"
#include "Components/ActorComponent.h"
#include "Containers/RingBuffer.h"
#include "LagCompensationComponent.generated.h"


USTRUCT()
struct FHitBoxHistory
{
	GENERATED_BODY()
	double Time;
	TMap<AActor*, TArray<FHitBox>> HitBoxes;
};


USTRUCT()
struct FPoseSnapshotHistory
{
	GENERATED_BODY()
	double Time;
	TMap<AActor*, FPoseSnapshot> PoseSnapShots;
	TMap<AActor*, FTransform> TransformSnapShots;

	bool IsValid() const
	{
		return PoseSnapShots.Num() > 0;
	}
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULagCompensationComponent();
	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void BeginRewind(float TimeStamp, const TArray<AActor*>& RewindTargets);
	void BeginRewind2(float TimeStamp, const TArray<AActor*>& RewindTargets);
	void SpawnDummies(const FHitBoxHistory& RewoundHistory, const TArray<AActor*>& RewindTargets);
	void SpawnDummies2(const FPoseSnapshotHistory& NearestFutureHistory, const FPoseSnapshotHistory& NearestPastHistory, const TArray<AActor*>& RewindTargets, float TargetTime);
	void EndRewind();
	void EndRewind2();

private:
	void RecordHitBoxHistories();
	void DrawHitBoxes(const TArray<FHitBox>& HitBoxes, FColor Color, bool bPersistentLines, float Time);

public:
	UPROPERTY(EditDefaultsOnly)
	double MaxRewindTime = 0.3;

private:
	TRingBuffer<FHitBoxHistory> HitBoxHistories;
	TRingBuffer<FPoseSnapshotHistory> PoseSnapshots;
	UPROPERTY()
	TArray<AActor*> RewindedDummies;
	UPROPERTY()
	TArray<USkeletalMeshComponent*> RewindedDummies2;

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UAnimInstance> DummyAnimInstanceClass;
};
