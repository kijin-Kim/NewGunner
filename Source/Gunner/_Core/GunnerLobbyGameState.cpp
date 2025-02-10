// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerLobbyGameState.h"

#include "GenericTeamAgentInterface.h"
#include "GunnerTeamAgentInterface.h"
#include "OnlineSessionSettings.h"
#include "GameFramework/PlayerState.h"
#include "Gunner/Gunner.h"
#include "Net/UnrealNetwork.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSession/GunnerSessionHelperSubsystem.h"

AGunnerLobbyGameState::AGunnerLobbyGameState()
{
	TeamBoxSlots.SetNum(10);
}

void AGunnerLobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGunnerLobbyGameState, MapName);
	DOREPLIFETIME(AGunnerLobbyGameState, LobbyName);
	DOREPLIFETIME(AGunnerLobbyGameState, TeamBoxSlots);
}

void AGunnerLobbyGameState::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		UGunnerSessionHelperSubsystem* SessionHelperSubsystem = GetSessionHelperSubsystem();
		check(SessionHelperSubsystem);
		IOnlineSessionPtr SessionInterface = SessionHelperSubsystem->GetSessionInterface();
		if (FNamedOnlineSession* NamedOnlineSession = SessionInterface->GetNamedSession(NAME_GameSession))
		{
			NamedOnlineSession->SessionSettings.Get(SETTING_SESSION_TEMPLATE_NAME, LobbyName);
			NamedOnlineSession->SessionSettings.Get(SETTING_MAPNAME, MapName);
			AuthSetLobbyName(LobbyName);
			AuthSetMapName(MapName);
		}
	}
}

void AGunnerLobbyGameState::AuthSetMapName(FString NewMapName)
{
	if (HasAuthority())
	{
		UGunnerSessionHelperSubsystem* SessionHelperSubsystem = GetSessionHelperSubsystem();
		check(SessionHelperSubsystem);
		if (FNamedOnlineSession* OnlineSession = SessionHelperSubsystem->GetSessionInterface()->GetNamedSession(NAME_GameSession))
		{
			OnlineSession->SessionSettings.Set(SETTING_MAPNAME, NewMapName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
			SessionHelperSubsystem->GetSessionInterface()->UpdateSession(NAME_GameSession, OnlineSession->SessionSettings, true);
		}
		MapName = NewMapName;
	}
}

void AGunnerLobbyGameState::AuthSetLobbyName(FString NewLobbyName)
{
	if (HasAuthority())
	{
		UGunnerSessionHelperSubsystem* SessionHelperSubsystem = GetSessionHelperSubsystem();
		check(SessionHelperSubsystem);
		if (FNamedOnlineSession* OnlineSession = SessionHelperSubsystem->GetSessionInterface()->GetNamedSession(NAME_GameSession))
		{
			OnlineSession->SessionSettings.Set(SETTING_SESSION_TEMPLATE_NAME, NewLobbyName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
			SessionHelperSubsystem->GetSessionInterface()->UpdateSession(NAME_GameSession, OnlineSession->SessionSettings, true);
		}
		LobbyName = NewLobbyName;
	}
}

void AGunnerLobbyGameState::AuthOnPlayerJoinedLobby(APlayerState* PlayerState)
{
	int32 SlotIndex = GetBalancedTeamBoxSlotIndex();
	check(!TeamBoxSlots[SlotIndex].IsValid())
	UGunnerSessionHelperSubsystem* SessionHelperSubsystem = GetSessionHelperSubsystem();
	check(SessionHelperSubsystem);
	TeamBoxSlots[SlotIndex].NickName = SessionHelperSubsystem->GetNickNameFromUniqueNetId(*PlayerState->GetUniqueId());
	TeamBoxSlots[SlotIndex].PingInMs = PlayerState->GetPingInMilliseconds();
	TeamBoxSlots[SlotIndex].UniqueNetId = PlayerState->GetUniqueId();
	AuthGrantTeamByIndex(PlayerState, SlotIndex);
	OnTeamBoxSlotsDirty.Broadcast();
}

void AGunnerLobbyGameState::AuthOnPlayerLeftLobby(APlayerState* PlayerState)
{
	for (int i = 0; i < TeamBoxSlots.Num(); i++)
	{
		if (TeamBoxSlots[i].UniqueNetId == PlayerState->GetUniqueId())
		{
			TeamBoxSlots[i].Reset();
		}
	}
	RearrangeTeamBoxSlots();
	OnTeamBoxSlotsDirty.Broadcast();
}

void AGunnerLobbyGameState::AuthChangeTeamBoxSlot(APlayerState* PlayerState)
{
	int32 CurrentSlotIndex = -1;
	
	for (int i = 0; i < TeamBoxSlots.Num(); i++)
	{
		if (TeamBoxSlots[i].UniqueNetId == PlayerState->GetUniqueId())
		{
			CurrentSlotIndex = i;
			break;
		}
	}
	check(CurrentSlotIndex != -1);

	int32 DestinationSlotIndex = -1;
	if (CurrentSlotIndex < 5)
	{
		for (int i = 5; i < 10; i++)
		{
			if (!TeamBoxSlots[i].IsValid())
			{
				TeamBoxSlots.Swap(CurrentSlotIndex, i);
				DestinationSlotIndex = i;
				break;
			}
		}
	}
	else
	{
		for (int i = 0; i < 5; i++)
		{
			if (!TeamBoxSlots[i].IsValid())
			{
				TeamBoxSlots.Swap(CurrentSlotIndex, i);
				DestinationSlotIndex = i;
				break;
			}
		}
	}
	AuthGrantTeamByIndex(PlayerState, DestinationSlotIndex);
	RearrangeTeamBoxSlots();
	OnTeamBoxSlotsDirty.Broadcast();
}

void AGunnerLobbyGameState::AuthGrantTeamByIndex(APlayerState* PlayerState, int32 Index)
{
	const bool bIsTeamGame = true;
	IGenericTeamAgentInterface* TeamAgentInterface = Cast<IGenericTeamAgentInterface>(PlayerState);
	check(TeamAgentInterface);
	TeamAgentInterface->SetGenericTeamId(bIsTeamGame ? (Index < 5 ? AttackerTeam : DefenderTeam) : FGenericTeamId::NoTeam);
}

FNormalizedTeamBoxSlots AGunnerLobbyGameState::GetNormalizedTeamBoxSlots() const
{
	FNormalizedTeamBoxSlots NormalizedTeamBoxSlots;
	for (int i = 0; i < 5; i++)
	{
		NormalizedTeamBoxSlots.LeftTeamSlots.Add(TeamBoxSlots[i]);
	}
	for (int i = 5; i < 10; i++)
	{
		NormalizedTeamBoxSlots.RightTeamSlots.Add(TeamBoxSlots[i]);
	}
	return NormalizedTeamBoxSlots;
}

void AGunnerLobbyGameState::OnRep_MapName()
{
	OnMapNameChangedReplicated.Broadcast(MapName);
}

void AGunnerLobbyGameState::OnRep_LobbyName()
{
	OnLobbyNameChangedReplicated.Broadcast(LobbyName);
}

void AGunnerLobbyGameState::OnRep_TeamBoxSlots()
{
	for (int i = 0; i < TeamBoxSlots.Num(); i++)
	{
		FTeamBoxSlot& Slot = TeamBoxSlots[i];
		if (Slot.IsValid())
		{
			UE_LOG(LogGunner, Verbose, TEXT("Slot %d: %s"), i, *Slot.NickName);
		}
		else
		{
			UE_LOG(LogGunner, Verbose, TEXT("Slot %d: Empty"), i);
		}
	}
	OnTeamBoxSlotsDirty.Broadcast();
}

UGunnerSessionHelperSubsystem* AGunnerLobbyGameState::GetSessionHelperSubsystem() const
{
	UGameInstance* GameInstance = GetGameInstance();
	return GameInstance->GetSubsystem<UGunnerSessionHelperSubsystem>();
}

int32 AGunnerLobbyGameState::GetBalancedTeamBoxSlotIndex() const
{
	FNormalizedTeamBoxSlots NormalizedTeamBoxSlots = GetNormalizedTeamBoxSlots();

	int32 LeftTeamCount = 0;
	for (int i = 0; i < NormalizedTeamBoxSlots.LeftTeamSlots.Num(); i++)
	{
		if (NormalizedTeamBoxSlots.LeftTeamSlots[i].IsValid())
		{
			LeftTeamCount++;
		}
	}

	int32 RightTeamCount = 0;
	for (int i = 0; i < NormalizedTeamBoxSlots.RightTeamSlots.Num(); i++)
	{
		if (NormalizedTeamBoxSlots.RightTeamSlots[i].IsValid())
		{
			RightTeamCount++;
		}
	}

	return LeftTeamCount <= RightTeamCount ? LeftTeamCount : 4 + RightTeamCount + 1;
}

void AGunnerLobbyGameState::RearrangeTeamBoxSlots()
{
	for (int i = 0; i < 5 - 1; ++i)
	{
		if (!TeamBoxSlots[i].IsValid())
		{
			TeamBoxSlots.Swap(i, i + 1);
		}
	}

	for (int i = 5; i < 10 - 1; ++i)
	{
		if (!TeamBoxSlots[i].IsValid())
		{
			TeamBoxSlots.Swap(i, i + 1);
		}
	}
}
