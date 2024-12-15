// Copyright Epic Games, Inc. All Rights Reserved.


#include "GunnerPlayerController.h"

#include "Gunner/_Core/UI/GunnerHUD.h"
#include "Gunner/_Core/Input/GunnerInputEventDispatcherComponent.h"
#include "Engine/Canvas.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerState.h"
#include "Gunner/Gunner.h"


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
		if (OnShowDebugInfoDelegateHandle.IsValid())
		{
			AHUD::OnShowDebugInfo.Remove(OnShowDebugInfoDelegateHandle);
		}
		OnShowDebugInfoDelegateHandle = AHUD::OnShowDebugInfo.AddUObject(this, &ThisClass::OnShowDebugInfo);

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
		
		if (OnShowDebugInfoDelegateHandle.IsValid())
		{
			AHUD::OnShowDebugInfo.Remove(OnShowDebugInfoDelegateHandle);
		}
		OnShowDebugInfoDelegateHandle = AHUD::OnShowDebugInfo.AddUObject(this, &ThisClass::OnShowDebugInfo);

		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			ServerRTT(GetWorld()->GetTimeSeconds());
		}, 1.0f, true, 0.0f);
	}
}

void AGunnerPlayerController::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& YL, float& YPos)
{
	if (HUD->GetCurrentDebugTargetActor() == GetPawn())
	{
		Canvas->DisplayDebugManager.DrawString(FString::Printf(TEXT("Ping: %f"), GetPlayerState<APlayerState>()->GetPingInMilliseconds()));
		Canvas->DisplayDebugManager.DrawString(FString::Printf(TEXT("RTT: %f"), RoundTripTime * 1000));
		Canvas->DisplayDebugManager.DrawString(FString::Printf(TEXT("Local Server Time: %f"), GetLocalServerTime()));
	}
}

double AGunnerPlayerController::GetLocalServerTime() const
{
	return GetWorld()->GetTimeSeconds() + ServerTimeDelta;
}

void AGunnerPlayerController::ServerRTT_Implementation(double ClientTime)
{
	ClientRTT(ClientTime, GetWorld()->GetTimeSeconds());
}

void AGunnerPlayerController::ClientRTT_Implementation(double ClientTime, double ServerTime)
{
	RoundTripTime = GetWorld()->GetTimeSeconds() - ClientTime;
	ServerTimeDelta = ServerTime - GetWorld()->GetTimeSeconds() + RoundTripTime * 0.5;
}
