// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerDeathMatchGameState.h"


TArray<int32> AGunnerDeathMatchGameState::DetermineWinners() const
{
	int32 MaxKills = 0;
	int32 WinnerID = 0;
	for (const FGunnerKillInfo& Info : KillInfos)
	{
		if (Info.Kills > MaxKills)
		{
			MaxKills = Info.Kills;
			WinnerID = Info.KillerPlayerId;
		}
	}
	return {WinnerID};
}
