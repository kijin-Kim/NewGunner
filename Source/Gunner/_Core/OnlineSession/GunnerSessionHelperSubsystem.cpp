// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerSessionHelperSubsystem.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Gunner/Gunner.h"
#include "Online/OnlineSessionNames.h"


#define DELEGATE_INITIALIZER(FunctionName) \
	FunctionName##Delegate(F##FunctionName##Delegate::CreateUObject(this, &ThisClass::FunctionName))


UGunnerSessionHelperSubsystem::UGunnerSessionHelperSubsystem() :
	DELEGATE_INITIALIZER(OnRegisterPlayersComplete),
	DELEGATE_INITIALIZER(OnUnregisterPlayersComplete),
	DELEGATE_INITIALIZER(OnCreateSessionComplete),
	DELEGATE_INITIALIZER(OnFindSessionsComplete),
	DELEGATE_INITIALIZER(OnCancelFindSessionsComplete),
	DELEGATE_INITIALIZER(OnJoinSessionComplete),
	DELEGATE_INITIALIZER(OnDestroySessionComplete),
	DELEGATE_INITIALIZER(OnSessionFailure),
	DELEGATE_INITIALIZER(OnUpdateSessionComplete)
{
}

void UGunnerSessionHelperSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	GEngine->NetworkFailureEvent.AddUObject(this, &UGunnerSessionHelperSubsystem::OnNetworkFailure);
	IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());
	if (OnlineSubsystem && OnlineSubsystem->GetSessionInterface())
	{
		IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
		SessionInterface->AddOnSessionFailureDelegate_Handle(OnSessionFailureDelegate);
		SessionInterface->AddOnRegisterPlayersCompleteDelegate_Handle(OnRegisterPlayersCompleteDelegate);
		SessionInterface->AddOnUnregisterPlayersCompleteDelegate_Handle(OnUnregisterPlayersCompleteDelegate);
		SessionInterface->AddOnUpdateSessionCompleteDelegate_Handle(OnUpdateSessionCompleteDelegate);
	}
}

void UGunnerSessionHelperSubsystem::Deinitialize()
{
	Super::Deinitialize();
	GEngine->NetworkFailureEvent.RemoveAll(this);
	IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());
	if (OnlineSubsystem && OnlineSubsystem->GetSessionInterface())
	{
		IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
		SessionInterface->ClearOnSessionFailureDelegate_Handle(OnSessionFailureDelegateHandle);
		SessionInterface->ClearOnRegisterPlayersCompleteDelegate_Handle(OnRegisterPlayersCompleteDelegateHandle);
		SessionInterface->ClearOnUnregisterPlayersCompleteDelegate_Handle(OnUnregisterPlayersCompleteDelegateHandle);
	}
}

void UGunnerSessionHelperSubsystem::CreateSession(FString LobbyName, FString MapName, int32 MaxNumPlayers)
{
	FOnlineSessionSettings OnlineSessionSettings;
	OnlineSessionSettings.bIsLANMatch = Online::GetSubsystem(GetWorld())->GetSubsystemName() == "NULL";
	OnlineSessionSettings.bUsesPresence = true;
	OnlineSessionSettings.NumPublicConnections = MaxNumPlayers;
	OnlineSessionSettings.NumPrivateConnections = 0;
	OnlineSessionSettings.bShouldAdvertise = true;
	OnlineSessionSettings.bAllowJoinInProgress = false;
	OnlineSessionSettings.bAllowJoinViaPresence = false;
	OnlineSessionSettings.Set(SETTING_SESSION_TEMPLATE_NAME, LobbyName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	OnlineSessionSettings.Set(SETTING_MAPNAME, MapName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);


	GetSessionInterface()->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);
	if (!GetSessionInterface()->CreateSession(0, NAME_GameSession, OnlineSessionSettings))
	{
		OnCreateSessionComplete(NAME_GameSession, false);
	}
}

void UGunnerSessionHelperSubsystem::FindSessions(FString LobbyName)
{
	OnlineSessionSearch = MakeShareable(new FOnlineSessionSearch());
	OnlineSessionSearch->bIsLanQuery = Online::GetSubsystem(GetWorld())->GetSubsystemName() == "NULL";
	OnlineSessionSearch->MaxSearchResults = 10;
	OnlineSessionSearch->PingBucketSize = 50;
	OnlineSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	OnlineSessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	SearchLobbyName = LobbyName;

	GetSessionInterface()->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);
	if (!GetSessionInterface()->FindSessions(0, OnlineSessionSearch.ToSharedRef()))
	{
		OnFindSessionsComplete(false);
	}
}

void UGunnerSessionHelperSubsystem::JoinSession(FString SessionIdStr)
{
	GetSessionInterface()->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);

	FOnlineSessionSearchResult* SessionSearchResultPtr = OnlineSessionSearch->SearchResults.FindByPredicate([SessionIdStr](const FOnlineSessionSearchResult& SearchResult)
	{
		return SearchResult.Session.GetSessionIdStr() == SessionIdStr;
	});
	if (!SessionSearchResultPtr)
	{
		OnJoinSessionComplete(NAME_GameSession, EOnJoinSessionCompleteResult::SessionDoesNotExist);
		return;
	}


	if (!GetSessionInterface()->JoinSession(0, NAME_GameSession, *SessionSearchResultPtr))
	{
		OnJoinSessionComplete(NAME_GameSession, EOnJoinSessionCompleteResult::UnknownError);
	}
}

void UGunnerSessionHelperSubsystem::DestroySession()
{
	GetSessionInterface()->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);

	if (!GetSessionInterface()->GetNamedSession(NAME_GameSession))
	{
		OnDestroySessionComplete(NAME_GameSession, false);
		return;
	}

	if (!GetSessionInterface()->DestroySession(NAME_GameSession))
	{
		OnDestroySessionComplete(NAME_GameSession, false);
	}
}

void UGunnerSessionHelperSubsystem::CancelFindSessions()
{
	GetSessionInterface()->AddOnCancelFindSessionsCompleteDelegate_Handle(OnCancelFindSessionsCompleteDelegate);
	if (!GetSessionInterface()->CancelFindSessions())
	{
		OnCancelFindSessionsComplete(false);
	}
}

IOnlineSessionPtr UGunnerSessionHelperSubsystem::GetSessionInterface() const
{
	return Online::GetSubsystem(GetWorld())->GetSessionInterface();
}

IOnlineIdentityPtr UGunnerSessionHelperSubsystem::GetIdentityInterface() const
{
	return Online::GetSubsystem(GetWorld())->GetIdentityInterface();
}

FString UGunnerSessionHelperSubsystem::BP_GetNickNameFromUniqueNetId(const FUniqueNetIdRepl& UniqueNetId) const
{
	return GetNickNameFromUniqueNetId(*UniqueNetId.GetUniqueNetId());
}

FString UGunnerSessionHelperSubsystem::GetNickNameFromUniqueNetId(const FUniqueNetId& UniqueNetId) const
{
	return GetIdentityInterface()->GetPlayerNickname(UniqueNetId);
}

TArray<FUniqueNetIdRepl> UGunnerSessionHelperSubsystem::BP_GetRegisteredPlayers() const
{
	TArray<FUniqueNetIdRepl> RegisteredPlayers;

	for (const FUniqueNetIdRef& Player : GetRegisteredPlayers())
	{
		RegisteredPlayers.Add(*Player);
	}
	return RegisteredPlayers;
}


TArray<FUniqueNetIdRef> UGunnerSessionHelperSubsystem::GetRegisteredPlayers() const
{
	TArray<FUniqueNetIdRef> RegisteredPlayers;
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (FNamedOnlineSession* NamedOnlineSession = SessionInterface->GetNamedSession(NAME_GameSession))
	{
		RegisteredPlayers = NamedOnlineSession->RegisteredPlayers;
	}
	return RegisteredPlayers;
}

FGunnerSessionLobbyInfo UGunnerSessionHelperSubsystem::GetCurrentLobbyInfo() const
{
	FGunnerSessionLobbyInfo LobbyInfo;
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (FNamedOnlineSession* NamedOnlineSession = SessionInterface->GetNamedSession(NAME_GameSession))
	{
		NamedOnlineSession->SessionSettings.Get(SETTING_SESSION_TEMPLATE_NAME, LobbyInfo.LobbyName);
		NamedOnlineSession->SessionSettings.Get(SETTING_MAPNAME, LobbyInfo.MapName);
		LobbyInfo.NumPublicConnections = NamedOnlineSession->SessionSettings.NumPublicConnections;
		LobbyInfo.NumOpenPublicConnections = NamedOnlineSession->NumOpenPublicConnections;
		LobbyInfo.OwningUserName = NamedOnlineSession->OwningUserName;
		LobbyInfo.SessionIdStr = NamedOnlineSession->GetSessionIdStr();
	}
	return LobbyInfo;
}

bool UGunnerSessionHelperSubsystem::IsHostPlayer(const FUniqueNetIdRepl& UniqueNetId) const
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (FNamedOnlineSession* NamedOnlineSession = SessionInterface->GetNamedSession(NAME_GameSession))
	{
		return NamedOnlineSession->OwningUserId == UniqueNetId;
	}
	return false;
}


void UGunnerSessionHelperSubsystem::OnRegisterPlayersComplete(FName SessionName, const TArray<FUniqueNetIdRef>& Players, bool bWasSuccessful)
{
	TArray<FUniqueNetIdRepl> Ids;
	for (const FUniqueNetIdRef& Player : Players)
	{
		Ids.Add(*Player);
	}
	OnRegisterPlayersCompleteDelegateMulticast.Broadcast(SessionName, Ids, bWasSuccessful);
}

void UGunnerSessionHelperSubsystem::OnUnregisterPlayersComplete(FName SessionName, const TArray<FUniqueNetIdRef>& Players, bool bWasSuccessful)
{
	TArray<FUniqueNetIdRepl> Ids;
	for (const FUniqueNetIdRef& Player : Players)
	{
		Ids.Add(*Player);
	}
	OnUnregisterPlayersCompleteDelegateMulticast.Broadcast(SessionName, Ids, bWasSuccessful);
}

void UGunnerSessionHelperSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	Online::GetSubsystem(GetWorld())->GetSessionInterface()->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
	OnCreateSessionCompleteDelegateMulticast.Broadcast(SessionName, bWasSuccessful);
}

void UGunnerSessionHelperSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	Online::GetSubsystem(GetWorld())->GetSessionInterface()->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);
	TArray<FGunnerSessionLobbyInfo> LobbyInfos;
	for (const FOnlineSessionSearchResult& SearchResult : OnlineSessionSearch->SearchResults)
	{
		FGunnerSessionLobbyInfo LobbyInfo;
		SearchResult.Session.SessionSettings.Get(SETTING_SESSION_TEMPLATE_NAME, LobbyInfo.LobbyName);
		if (SearchLobbyName != TEXT("None") && !LobbyInfo.LobbyName.Contains(SearchLobbyName))
		{
			continue;
		}

		SearchResult.Session.SessionSettings.Get(SETTING_MAPNAME, LobbyInfo.MapName);
		LobbyInfo.PingInMs = SearchResult.PingInMs;
		LobbyInfo.NumPublicConnections = SearchResult.Session.SessionSettings.NumPublicConnections;
		LobbyInfo.NumOpenPublicConnections = SearchResult.Session.NumOpenPublicConnections;
		LobbyInfo.OwningUserName = SearchResult.Session.OwningUserName;
		LobbyInfo.SessionIdStr = SearchResult.Session.GetSessionIdStr();
		LobbyInfos.Add(LobbyInfo);
	}
	SearchLobbyName = TEXT("None");
	OnFindSessionsCompleteDelegateMulticast.Broadcast(bWasSuccessful, LobbyInfos);
}

void UGunnerSessionHelperSubsystem::OnCancelFindSessionsComplete(bool bWasSuccessful)
{
	Online::GetSubsystem(GetWorld())->GetSessionInterface()->ClearOnCancelFindSessionsCompleteDelegate_Handle(OnCancelFindSessionsCompleteDelegateHandle);
	OnCancelFindSessionsCompleteDelegateMulticast.Broadcast(bWasSuccessful);
}

void UGunnerSessionHelperSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	Online::GetSubsystem(GetWorld())->GetSessionInterface()->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);
	OnJoinSessionCompleteDelegateMulticast.Broadcast(SessionName, LexToString(Result));
}

void UGunnerSessionHelperSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	Online::GetSubsystem(GetWorld())->GetSessionInterface()->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);
	OnDestroySessionCompleteDelegateMulticast.Broadcast(SessionName, bWasSuccessful);
}

void UGunnerSessionHelperSubsystem::OnSessionFailure(const FUniqueNetId& UniqueNetId, ESessionFailure::Type FailureType)
{
	OnSessionFailureDelegateMulticast.Broadcast(UniqueNetId, LexToString(FailureType));
}

void UGunnerSessionHelperSubsystem::OnUpdateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	OnUpdateSessionCompleteDelegateMulticast.Broadcast(SessionName, bWasSuccessful);
}

void UGunnerSessionHelperSubsystem::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& FailureString)
{
	GR_LOG_SUB(World->GetFirstPlayerController(), LogGunnerSession, Error, TEXT("네트워크 실패: FailureType=%s, FailureString=%s"), ToString(FailureType), *FailureString);
	DestroySession();
}
