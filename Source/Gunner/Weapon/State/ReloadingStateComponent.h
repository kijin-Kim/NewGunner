// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateComponent.h"
#include "ReloadingStateComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UReloadingStateComponent : public UStateComponent
{
	GENERATED_BODY()

public:
	UReloadingStateComponent();
	
	virtual void Enter() override;
	virtual void Exit() override;
	
	void LocalReload();
	UFUNCTION(Server, Reliable)
	void ServerReload();
};
