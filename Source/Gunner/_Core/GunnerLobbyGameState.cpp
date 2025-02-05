// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerLobbyGameState.h"

#include "OnlineSessionSettings.h"
#include "Net/UnrealNetwork.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSession/GunnerSessionHelperSubsystem.h"

void AGunnerLobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGunnerLobbyGameState, MapName);
	DOREPLIFETIME(AGunnerLobbyGameState, LobbyName);
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

void AGunnerLobbyGameState::OnRep_MapName()
{
	OnMapNameChangedReplicated.Broadcast(MapName);
}

void AGunnerLobbyGameState::OnRep_LobbyName()
{
	OnLobbyNameChangedReplicated.Broadcast(LobbyName);
}

UGunnerSessionHelperSubsystem* AGunnerLobbyGameState::GetSessionHelperSubsystem() const
{
	UGameInstance* GameInstance = GetGameInstance();
	return GameInstance->GetSubsystem<UGunnerSessionHelperSubsystem>();
}
