// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateComponent.h"
#include "InventoriedStateComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UInventoriedStateComponent : public UStateComponent
{
	GENERATED_BODY()

public:
	UInventoriedStateComponent();
	virtual void Enter() override;
};
