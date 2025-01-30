// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/PlayerState.h"
#include "GunnerPlayerState.generated.h"



class UGunnerActionSetupComponent;
class UGunnerEventManagerComponent;
class UGunnerActionComponent;
/**
 * 
 */
UCLASS()
class GUNNER_API AGunnerPlayerState : public APlayerState, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	AGunnerPlayerState();
	virtual void PostInitializeComponents() override;
	virtual void SetGenericTeamId(const FGenericTeamId& InTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamID; }


private:
	UFUNCTION()
	void OnPawnSetEvent(APlayerState* Player, APawn* NewPawn, APawn* OldPawn);
	
	

private:
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = true))
	TObjectPtr<UGunnerActionComponent> ActionComponent;
	UPROPERTY()
	TObjectPtr<UGunnerEventManagerComponent> EventManagerComponent;
	UPROPERTY(Replicated)
	FGenericTeamId TeamID;
};
