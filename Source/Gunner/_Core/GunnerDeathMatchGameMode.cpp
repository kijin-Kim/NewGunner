// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerDeathMatchGameMode.h"
#include "GunnerDeathMatchGameState.h"

AGunnerDeathMatchGameMode::AGunnerDeathMatchGameMode()
{
	GameStateClass = AGunnerDeathMatchGameState::StaticClass();
}

void AGunnerDeathMatchGameMode::AuthRegisterKill(AController* Killer, AController* Victim, FName KillCauserName)
{
	Super::AuthRegisterKill(Killer, Victim, KillCauserName);
	AGunnerGameState* GS = GetGameState<AGunnerGameState>();
	FGunnerKillInfo* KillInfo = GS->GetKillerInfo(Killer);
	if (KillInfo && KillInfo->Kills >= KillLimit)
	{
		EndMatch();
		return;
	}
	
	if (Victim)
	{
		RestartPlayer(Victim);
	}
}
