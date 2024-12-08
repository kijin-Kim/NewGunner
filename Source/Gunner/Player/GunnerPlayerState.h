// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Gunner/_Core/ActionSystem/GunnerActionDefinition.h"
#include "GunnerPlayerState.generated.h"

class UGunnerEventManagerComponent;
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

private:
	UFUNCTION()
	void OnPawnSetEvent(APlayerState* Player, APawn* NewPawn, APawn* OldPawn);
	
	

private:
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = true))
	TObjectPtr<UGunnerActionComponent> ActionComponent;
	UPROPERTY()
	TObjectPtr<UGunnerEventManagerComponent> EventManagerComponent;
};
