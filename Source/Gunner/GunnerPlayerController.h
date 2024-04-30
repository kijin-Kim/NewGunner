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
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnRep_PlayerState() override;
	void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& YL, float& YPos);
	
	double GetLocalServerTime() const;

private:
	UFUNCTION(Server, Reliable)
	void ServerRTT(double ClientTime);
	UFUNCTION(Client, Reliable)
	void ClientRTT(double ClientTime, double ServerTime);
	
	
private:
	double RoundTripTime = 0.0f;
	double ServerTimeDelta = 0.0f;
	FDelegateHandle OnShowDebugInfoDelegateHandle;
};
