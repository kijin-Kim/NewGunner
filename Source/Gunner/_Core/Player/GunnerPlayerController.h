// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/PlayerController.h"
#include "GunnerPlayerController.generated.h"


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
	virtual void DisplayDebug(class UCanvas* Canvas, const class FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;
	virtual void InitPlayerState() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnRep_PlayerState() override;

private:
	void TrySetParameterCollectionLocalPlayerTeamID();
	void SetParameterCollectionLocalPlayerTeamID(FGenericTeamId TeamID);
	void CreateMainWidget();

protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> MainWidgetClass;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UGunnerInputEventDispatcherComponent> InputEventDispatcherComponent;

	UPROPERTY()
	TObjectPtr<UUserWidget> MainWidget;
};
