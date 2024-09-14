// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Gunner/Core/ActionSystem/GunnerActionDefinition.h"
#include "GunnerPlayerState.generated.h"

class UGunnerActionComponent;
/**
 * 
 */
UCLASS()
class GUNNER_API AGunnerPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	AGunnerPlayerState();
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void ClientInitialize(AController* C) override;
	void SetupActionComponent();

private:
	UPROPERTY()
	TObjectPtr<UGunnerActionComponent> ActionComponent;
	
};
