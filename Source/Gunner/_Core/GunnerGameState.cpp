// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerGameState.h"

#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

void AGunnerGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AGunnerGameState, MatchTimeLimit, COND_InitialOnly);
	DOREPLIFETIME(AGunnerGameState, PlayerKills);
}

FGunnerPlayerKillInfo* AGunnerGameState::GetKillerInfo(AController* Killer)
{
	return PlayerKills.FindByPredicate([Killer](const FGunnerPlayerKillInfo& Info)
	{
		return Info.PlayerId == Killer->PlayerState->GetPlayerId();
	});
}

void AGunnerGameState::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();
	NetMulticastBroadcastWinners(DetermineWinners());
}

void AGunnerGameState::NetMulticastBroadcastWinners_Implementation(const TArray<int32>& WinnerIds)
{
	OnMatchEndedDelegate.Broadcast(WinnerIds);
}

void AGunnerGameState::AuthRegisterKill(AController* Killer, AController* Victim)
{
	check(HasAuthority());
	if (FGunnerPlayerKillInfo* KillerInfo = GetKillerInfo(Killer))
	{
		KillerInfo->Kills++;
		return;
	}

	FGunnerPlayerKillInfo NewInfo;
	NewInfo.PlayerId = Killer->PlayerState->GetPlayerId();
	NewInfo.Kills = 1;
	PlayerKills.Add(NewInfo);
}
