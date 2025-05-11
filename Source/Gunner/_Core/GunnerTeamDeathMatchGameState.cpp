// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerTeamDeathMatchGameState.h"

void AGunnerTeamDeathMatchGameState::AddTeamKillCount(FGenericTeamId TeamId, int32 KillCount)
{
	KillCountPerTeam.FindOrAdd(TeamId)++;
	OnTeamKillCountChanged.Broadcast(TeamId, KillCountPerTeam[TeamId]);
}

void AGunnerTeamDeathMatchGameState::OnRep_KillInfos()
{
	Super::OnRep_KillInfos();

	TMap<FGenericTeamId, int32> OldKillCountPerTeam = KillCountPerTeam;

	KillCountPerTeam.Empty();
	for (const FGunnerKillInfo& Info : GetKillInfos())
	{
		KillCountPerTeam.FindOrAdd(Info.TeamId) += Info.Kills;
	}

	for (const auto& [TeamID, Count] : KillCountPerTeam)
	{
		if (OldKillCountPerTeam.Contains(TeamID))
		{
			int32 OldCount = OldKillCountPerTeam[TeamID];
			if (OldCount != Count)
			{
				OnTeamKillCountChanged.Broadcast(TeamID, Count);
			}
		}
		else
		{
			OnTeamKillCountChanged.Broadcast(TeamID, Count);
		}
	}
}

void AGunnerTeamDeathMatchGameState::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();
	for (const auto& [TeamID, Count] : KillCountPerTeam)
	{
		OnTeamKillCountChanged.Broadcast(TeamID, 0);
	}
	KillCountPerTeam.Empty();
}
