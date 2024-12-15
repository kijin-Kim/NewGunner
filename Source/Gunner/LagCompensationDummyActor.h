// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "_Core/HitBoxActorInterface.h"
#include "LagCompensationDummyActor.generated.h"

UCLASS()
class GUNNER_API ALagCompensationDummyActor : public AActor, public IHitBoxActorInterface
{
	GENERATED_BODY()

public:
	ALagCompensationDummyActor();
	virtual TArray<FHitBox> CollectAndGetHitBoxes() override;

};
