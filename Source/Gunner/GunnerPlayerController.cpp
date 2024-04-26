// Copyright Epic Games, Inc. All Rights Reserved.


#include "GunnerPlayerController.h"

#include "Gunner.h"
#include "GameFramework/PlayerState.h"

void AGunnerPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AGunnerPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ClientSendServerTime(GetWorld()->GetTimeSeconds());
}

void AGunnerPlayerController::ClientSendServerTime_Implementation(float ServerTime)
{
	ServerTimeDelta = ServerTime - GetWorld()->GetTimeSeconds();
}
