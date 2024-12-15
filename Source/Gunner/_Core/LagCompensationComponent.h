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

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULagCompensationComponent();
	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void BeginRewind(float TimeStamp, const TArray<AActor*>& RewindTargets);
	void SpawnDummies(const FHitBoxHistory& RewoundHistory, const TArray<AActor*>& RewindTargets);
	void EndRewind();


private:
	void RecordHitBoxHistories();
	void DrawHitBoxes(const TArray<FHitBox>& HitBoxes, FColor Color, bool bPersistentLines, float Time);

public:
	UPROPERTY(EditDefaultsOnly)
	double MaxRewindTime = 0.3;

private:
	TRingBuffer<FHitBoxHistory> HitBoxHistories;
	UPROPERTY()
	TArray<AActor*> RewindedDummies;
};
