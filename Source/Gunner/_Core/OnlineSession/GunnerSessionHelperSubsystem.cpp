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
	DELEGATE_INITIALIZER(OnJoinSessionComplete),
	DELEGATE_INITIALIZER(OnDestroySessionComplete),
	DELEGATE_INITIALIZER(OnSessionFailure)
{
}

void UGunnerSessionHelperSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	GEngine->NetworkFailureEvent.AddUObject(this, &UGunnerSessionHelperSubsystem::OnNetworkFailure);
}

void UGunnerSessionHelperSubsystem::Deinitialize()
{
	Super::Deinitialize();
	GEngine->NetworkFailureEvent.RemoveAll(this);
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
	OnlineSessionSettings.Settings.Add(SETTING_SESSION_TEMPLATE_NAME, LobbyName);
	OnlineSessionSettings.Settings.Add(SETTING_MAPNAME, MapName);


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
	OnlineSessionSearch->PingBucketSize = 10;
	OnlineSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	OnlineSessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	OnlineSessionSearch->QuerySettings.Set(SEARCH_MINSLOTSAVAILABLE, 1, EOnlineComparisonOp::Equals);
	//OnlineSessionSearch->QuerySettings.Set(SEARCH_KEYWORDS, LobbyName, EOnlineComparisonOp::Equals);

	GetSessionInterface()->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);
	if (!GetSessionInterface()->FindSessions(0, OnlineSessionSearch.ToSharedRef()))
	{
		OnFindSessionsComplete(false);
	}
}

void UGunnerSessionHelperSubsystem::JoinSession(int32 SessionResultIndex)
{
	GetSessionInterface()->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);
	CancelFindSessions();

	if (!OnlineSessionSearch->SearchResults.IsValidIndex(SessionResultIndex))
	{
		OnJoinSessionComplete(NAME_GameSession, EOnJoinSessionCompleteResult::SessionDoesNotExist);
		return;
	}

	if (!GetSessionInterface()->JoinSession(0, NAME_GameSession, OnlineSessionSearch->SearchResults[SessionResultIndex]))
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
	if (OnlineSessionSearch->SearchState == EOnlineAsyncTaskState::InProgress)
	{
		GetSessionInterface()->CancelFindSessions();
	}
}

void UGunnerSessionHelperSubsystem::EnableTraceSessionFailure(bool bEnable)
{
	if (bEnable)
	{
		GetSessionInterface()->AddOnSessionFailureDelegate_Handle(OnSessionFailureDelegate);
	}
	else
	{
		GetSessionInterface()->ClearOnSessionFailureDelegate_Handle(OnSessionFailureDelegateHandle);
	}
}

void UGunnerSessionHelperSubsystem::EnableTraceRegisteredPlayers(bool bEnable)
{
	if (bEnable)
	{
		GetSessionInterface()->AddOnRegisterPlayersCompleteDelegate_Handle(OnRegisterPlayersCompleteDelegate);
		GetSessionInterface()->AddOnUnregisterPlayersCompleteDelegate_Handle(OnUnregisterPlayersCompleteDelegate);
	}
	else
	{
		GetSessionInterface()->ClearOnRegisterPlayersCompleteDelegate_Handle(OnRegisterPlayersCompleteDelegateHandle);
		GetSessionInterface()->ClearOnUnregisterPlayersCompleteDelegate_Handle(OnUnregisterPlayersCompleteDelegateHandle);
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
	OnFindSessionsCompleteDelegateMulticast.Broadcast(bWasSuccessful);
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

void UGunnerSessionHelperSubsystem::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& FailureString)
{
	UE_LOG(LogGunner, Error, TEXT("Network Failure: %s, %s"), ToString(FailureType), *FailureString);
}
