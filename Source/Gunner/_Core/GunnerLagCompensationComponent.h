// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Containers/RingBuffer.h"
#include "GunnerLagCompensationComponent.generated.h"


class URewoundSnapshotAnimInstance;

USTRUCT()
struct FMyPoseSnapshot
{
	GENERATED_BODY()
	double Time;
	FPoseSnapshot PoseSnapshot;
	FTransform TransformSnapshot;

	bool IsValid() const
	{
		return PoseSnapshot.LocalTransforms.Num() > 0
			&& PoseSnapshot.BoneNames.Num() > 0;
	}
};


UCLASS(Config=Game, DefaultConfig, meta=(BlueprintSpawnableComponent))
class GUNNER_API UGunnerLagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGunnerLagCompensationComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	bool AuthBeginRewind(float TimeStamp, double& OutRewoundedTimeStamp);
	void AuthEndRewind();

private:
	void AuthSpawnDummyMesh(const FMyPoseSnapshot& NearestFutureSnapshot, const FMyPoseSnapshot& NearestPastSnapshot, float TargetTime);

private:
	UPROPERTY(Config)
	TSubclassOf<URewoundSnapshotAnimInstance> PoseSnapshotAnimInstanceClass;
	UPROPERTY(Config)
	double MaxRewindTime = 0.3;
	

	UPROPERTY()
	ACharacter* CharacterOwner;

	TRingBuffer<FMyPoseSnapshot> PoseSnapshots;

	UPROPERTY()
	USkeletalMeshComponent* DummyMeshComponent;
};

