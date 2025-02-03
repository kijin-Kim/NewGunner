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
	virtual void PreInitializeComponents() override;


private:
	UFUNCTION()
	void OnUpdateSessionComplete(FName SessionName, bool bWasSuccessful);

	UFUNCTION(Client, Reliable)
	void NotifyClientUpdateSessionComplete(FName SessionName, bool bWasSuccessful);
	
	UFUNCTION()
	void OnFindSessionsComplete(bool bWasSuccessful, const TArray<FGunnerSessionLobbyInfo>& LobbyInfos);
};
