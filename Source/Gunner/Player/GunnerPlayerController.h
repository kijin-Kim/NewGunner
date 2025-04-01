// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/PlayerController.h"
#include "GunnerPlayerController.generated.h"


class UGunnerInputEventDispatcherComponent;
/**
 *
 */
UCLASS()
class GUNNER_API AGunnerPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AGunnerPlayerController();
	virtual void OnPossess(APawn* InPawn) override;
	virtual void InitPlayerState() override;
	virtual void OnRep_PlayerState() override;

private:
	void TrySetParameterCollectionLocalPlayerTeamID();
	void SetParameterCollectionLocalPlayerTeamID(FGenericTeamId TeamID);

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UGunnerInputEventDispatcherComponent> InputEventDispatcherComponent;
};
