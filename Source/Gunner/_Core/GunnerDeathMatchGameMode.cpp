// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerDeathMatchGameMode.h"
#include "GunnerDeathMatchGameState.h"
#include "GameFramework/PlayerState.h"

AGunnerDeathMatchGameMode::AGunnerDeathMatchGameMode()
{
	GameStateClass = AGunnerDeathMatchGameState::StaticClass();
}

void AGunnerDeathMatchGameMode::AuthRegisterKill(AController* Killer, AController* Victim, FName KillCauserName)
{
	Super::AuthRegisterKill(Killer, Victim, KillCauserName);
	AGunnerGameState* GS = GetGameState<AGunnerGameState>();
	if (GS->GetKillerInfo(Killer)->Kills >= KillLimit)
	{
		EndMatch();
	};
}
