// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerLobbyPlayerController.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSession/GunnerSessionHelperSubsystem.h"


void AGunnerLobbyPlayerController::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	if(!GetGameInstance())
	{
		return;
	}
	if (UGunnerSessionHelperSubsystem* SessionHelperSubsystem = GetGameInstance()->GetSubsystem<UGunnerSessionHelperSubsystem>())
	{
		SessionHelperSubsystem->OnUpdateSessionCompleteDelegateMulticast.AddDynamic(this, &AGunnerLobbyPlayerController::OnUpdateSessionComplete);
	}
}

void AGunnerLobbyPlayerController::OnUpdateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UGunnerSessionHelperSubsystem* SessionHelperSubsystem = GetGameInstance()->GetSubsystem<UGunnerSessionHelperSubsystem>();
	IOnlineSessionPtr SessionInterface = SessionHelperSubsystem->GetSessionInterface();
	FNamedOnlineSession* CurrentSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (!CurrentSession || !HasAuthority() || IsLocalController() || Online::GetSubsystem(GetWorld())->GetSubsystemName() != "NULL")
	{
		return;
	}
	
	NotifyClientUpdateSessionComplete(SessionName, bWasSuccessful);
}

void AGunnerLobbyPlayerController::OnFindSessionsComplete(bool bWasSuccessful, const TArray<FGunnerSessionLobbyInfo>& LobbyInfos)
{
	UGunnerSessionHelperSubsystem* SessionHelperSubsystem = GetGameInstance()->GetSubsystem<UGunnerSessionHelperSubsystem>();
	IOnlineSessionPtr SessionInterface = SessionHelperSubsystem->GetSessionInterface();
	FNamedOnlineSession* NamedOnlineSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (!NamedOnlineSession)
	{
		return;
	}

	SessionHelperSubsystem->OnFindSessionsCompleteDelegateMulticast.RemoveDynamic(this, &AGunnerLobbyPlayerController::OnFindSessionsComplete);
	TSharedPtr<FOnlineSessionSearch> SessionSearch = SessionHelperSubsystem->GetOnlineSessionSearch();

	for (FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
	{
		if (SearchResult.GetSessionIdStr() == NamedOnlineSession->GetSessionIdStr())
		{
			SessionInterface->UpdateSession(NAME_GameSession, SearchResult.Session.SessionSettings, false);
		}
	}
}

void AGunnerLobbyPlayerController::NotifyClientUpdateSessionComplete_Implementation(FName SessionName, bool bWasSuccessful)
{
	if (UGunnerSessionHelperSubsystem* SessionHelperSubsystem = GetGameInstance()->GetSubsystem<UGunnerSessionHelperSubsystem>())
	{
		SessionHelperSubsystem->OnFindSessionsCompleteDelegateMulticast.AddDynamic(this, &AGunnerLobbyPlayerController::OnFindSessionsComplete);
		SessionHelperSubsystem->FindSessions();
		SessionHelperSubsystem->OnUpdateSessionCompleteDelegateMulticast.Broadcast(SessionName, bWasSuccessful);
	}
}
