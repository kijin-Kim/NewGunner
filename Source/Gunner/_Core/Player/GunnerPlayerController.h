// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/PlayerController.h"
#include "GunnerPlayerController.generated.h"


DECLARE_MULTICAST_DELEGATE_OneParam(FOnGunnerPlayerControlRotationChangedSignature, const FRotator& /*NewRotation*/);


class UGunnerOverlayWidget;
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
	virtual void SetControlRotation(const FRotator& NewRotation) override;
	virtual void DisplayDebug(class UCanvas* Canvas, const class FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;
	virtual void InitPlayerState() override;
	virtual void OnRep_PlayerState() override;

public:
	FOnGunnerPlayerControlRotationChangedSignature OnGunnerPlayerControlRotationChangedDelegate;

private:
	void TrySetParameterCollectionLocalPlayerTeamID();
	void SetParameterCollectionLocalPlayerTeamID(FGenericTeamId TeamID);

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UGunnerInputEventDispatcherComponent> InputEventDispatcherComponent;
};
