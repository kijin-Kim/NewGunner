// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerLobbyPlayerState.h"

#include "Net/UnrealNetwork.h"

AGunnerLobbyPlayerState::AGunnerLobbyPlayerState()
{
	bReplicates = true;
}

void AGunnerLobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(AGunnerLobbyPlayerState, TeamID, COND_None, REPNOTIFY_Always);
}

void AGunnerLobbyPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);
	IGunnerTeamAgentInterface* TeamAgentInterface = Cast<IGunnerTeamAgentInterface>(PlayerState);
	if (TeamAgentInterface)
	{
		TeamAgentInterface->SetGenericTeamId(GetGenericTeamId());
	}
}

void AGunnerLobbyPlayerState::SetGenericTeamId(const FGenericTeamId& InTeamID)
{
	FGenericTeamId OldTeamID = TeamID;
	TeamID = InTeamID;
	OnTeamSet.Broadcast(OldTeamID, TeamID);
}

void AGunnerLobbyPlayerState::OnRep_TeamID(FGenericTeamId OldTeamID)
{
	OnTeamSet.Broadcast(OldTeamID, TeamID);
}
