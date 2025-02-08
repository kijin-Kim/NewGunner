// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GunnerLobbyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class GUNNER_API AGunnerLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void ChangeTeamBoxSlot();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void SetupTeamBoxSlotEvent();
	
	virtual void OnRep_PlayerState() override;

private:
	UFUNCTION(Server, Reliable)
	void ServerChangeTeamBoxSlot(APlayerState* InPlayerState);
};
