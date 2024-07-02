// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateComponent.h"
#include "FiringStateComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UFiringStateComponent : public UStateComponent
{
	GENERATED_BODY()

public:
	UFiringStateComponent();
	virtual void Enter() override;
	virtual void Exit() override;
	void PlayFireMontage();
	virtual void LocalFire();
	UFUNCTION(Server, Reliable)
	void ServerFire();

	

};
