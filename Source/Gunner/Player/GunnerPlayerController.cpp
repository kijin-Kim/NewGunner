// Copyright Epic Games, Inc. All Rights Reserved.


#include "GunnerPlayerController.h"

#include "GameFramework/HUD.h"
#include "Gunner/_Core/Input/GunnerInputEventDispatcherComponent.h"
#include "Gunner/_Core/UI/GunnerHUD.h"


AGunnerPlayerController::AGunnerPlayerController()
{
	InputEventDispatcherComponent = CreateDefaultSubobject<UGunnerInputEventDispatcherComponent>(TEXT("InputEventDispatcher"));
	bAutoManageActiveCameraTarget = false;
}

void AGunnerPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (IsLocalController())
	{
		AGunnerHUD* GunnerHUD = Cast<AGunnerHUD>(GetHUD());
		check(GunnerHUD)
		GunnerHUD->SetupHUD(PlayerState);
	}
}

void AGunnerPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	if (IsLocalController())
	{
		AGunnerHUD* GunnerHUD = Cast<AGunnerHUD>(GetHUD());
		check(GunnerHUD)
		GunnerHUD->SetupHUD(PlayerState);
	}
}

