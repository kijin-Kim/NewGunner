// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GunnerPlayerController.generated.h"

class UInputMappingContext;

/**
 *
 */
UCLASS()
class GUNNER_API AGunnerPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION(Client, Reliable)
	void ClientSendServerTime(float ServerTime);
	
private:
	float ServerTimeDelta = 0.0f;
};
