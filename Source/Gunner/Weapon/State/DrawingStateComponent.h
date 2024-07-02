// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateComponent.h"
#include "DrawingStateComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UDrawingStateComponent : public UStateComponent
{
	GENERATED_BODY()

public:
	UDrawingStateComponent();
	virtual void Enter() override;
	virtual void Exit() override;
	void OnWeaponEquipEnded(UAnimMontage* AnimMontage, bool bInterrupted);
};
