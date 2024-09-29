// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Gunner/Core/ActionSystem/GunnerActionDefinition.h"
#include "GunnerPlayerState.generated.h"

class UEventManagerComponent;
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
	virtual void ClientInitialize(AController* C) override;

private:
	void SetupOnPossessedPawnChangedEvent();
	UFUNCTION()
	void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);
	

private:
	UPROPERTY()
	TObjectPtr<UGunnerActionComponent> ActionComponent;
	UPROPERTY()
	TObjectPtr<UEventManagerComponent> EventManagerComponent;

	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<UGunnerAction>> TestActionClasses;
};
