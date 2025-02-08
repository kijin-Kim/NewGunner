// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerLobbyPlayerController.h"

#include "GunnerLobbyGameState.h"
#include "GameFramework/PlayerState.h"
#include "Gunner/Gunner.h"


void AGunnerLobbyPlayerController::ChangeTeamBoxSlot()
{
	if (HasAuthority())
	{
		AGunnerLobbyGameState* GS = GetWorld()->GetGameState<AGunnerLobbyGameState>();
		check(GS);
		GS->AuthChangeTeamBoxSlot(PlayerState);
	}
	else
	{
		ServerChangeTeamBoxSlot(PlayerState);
	}
}

void AGunnerLobbyPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	GR_LOG(LogGunner, Verbose, TEXT("PlayerState: %s"), *PlayerState->GetPlayerName());
	SetupTeamBoxSlotEvent();
}

void AGunnerLobbyPlayerController::ServerChangeTeamBoxSlot_Implementation(APlayerState* InPlayerState)
{
	ChangeTeamBoxSlot();
}
