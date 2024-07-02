// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateComponent.h"
#include "DroppedStateComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UDroppedStateComponent : public UStateComponent
{
	GENERATED_BODY()

public:
	UDroppedStateComponent();
	virtual void Enter() override;
};
