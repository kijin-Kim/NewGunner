// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerLobbyGameMode.h"

#include "GunnerLobbyGameState.h"
#include "GameFramework/PlayerState.h"
#include "Gunner/Gunner.h"

void AGunnerLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	GR_VLOG_FN(this, LogGunnerSession, Display, TEXT("PlayerName=%s"), *NewPlayer->PlayerState->GetPlayerName());
	if (AGunnerLobbyGameState* GS = GetGameState<AGunnerLobbyGameState>())
	{
		GS->AuthOnPlayerJoinedLobby(NewPlayer->PlayerState);
	}
}

void AGunnerLobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	GR_VLOG_FN(this, LogGunnerSession, Display, TEXT("PlayerName=%s"), *Exiting->PlayerState->GetPlayerName());
	if (AGunnerLobbyGameState* GS = GetGameState<AGunnerLobbyGameState>())
	{
		GS->AuthOnPlayerLeftLobby(Exiting->PlayerState);
	}
}
