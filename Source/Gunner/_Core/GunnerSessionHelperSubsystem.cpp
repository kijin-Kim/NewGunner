// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerSessionHelperSubsystem.h"

#include "Online/OnlineSessionNames.h"

#define DELEGATE_INITIALIZER(FunctionName) \
	FunctionName##Delegate(F##FunctionName##Delegate::CreateUObject(this, &ThisClass::FunctionName))

UGunnerSessionHelperSubsystem::UGunnerSessionHelperSubsystem() :
	DELEGATE_INITIALIZER(OnSessionParticipantJoined),
	DELEGATE_INITIALIZER(OnSessionParticipantLeft),
	DELEGATE_INITIALIZER(OnCreateSessionComplete),
	DELEGATE_INITIALIZER(OnFindSessionsComplete),
	DELEGATE_INITIALIZER(OnJoinSessionComplete)

{
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

	if (!GetSessionInterface()->CreateSession(0, NAME_GameSession, OnlineSessionSettings))
	{
		OnCreateSessionComplete(NAME_GameSession, false);
	}
}

void UGunnerSessionHelperSubsystem::FindLobbies(FString LobbyName)
{
	OnlineSessionSearch = MakeShareable(new FOnlineSessionSearch());
	OnlineSessionSearch->bIsLanQuery = Online::GetSubsystem(GetWorld())->GetSubsystemName() == "NULL";
	OnlineSessionSearch->MaxSearchResults = 10;
	OnlineSessionSearch->PingBucketSize = 10;
	OnlineSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	OnlineSessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	OnlineSessionSearch->QuerySettings.Set(SEARCH_MINSLOTSAVAILABLE, 1, EOnlineComparisonOp::Equals);
	//OnlineSessionSearch->QuerySettings.Set(SEARCH_KEYWORDS, LobbyName, EOnlineComparisonOp::Equals);

	if (!GetSessionInterface()->FindSessions(0, OnlineSessionSearch.ToSharedRef()))
	{
		OnFindSessionsComplete(false);
	}
}

void UGunnerSessionHelperSubsystem::JoinSession()
{
	if (!GetSessionInterface()->JoinSession(0, NAME_GameSession, OnlineSessionSearch->SearchResults[0]))
	{
		OnJoinSessionComplete(NAME_GameSession, EOnJoinSessionCompleteResult::UnknownError);
	}
}

void UGunnerSessionHelperSubsystem::LeaveSession()
{
	GetSessionInterface()->DestroySession(NAME_GameSession);
}

IOnlineSessionPtr UGunnerSessionHelperSubsystem::GetSessionInterface() const
{
	return Online::GetSubsystem(GetWorld())->GetSessionInterface();
}

void UGunnerSessionHelperSubsystem::OnSessionParticipantJoined(FName Name, const FUniqueNetId& UniqueNetId)
{
	Online::GetSubsystem(GetWorld())->GetSessionInterface()->ClearOnSessionParticipantJoinedDelegate_Handle(OnSessionParticipantJoinedDelegateHandle);
	OnSessionParticipantJoinedDelegateMulticast.Broadcast(Name, UniqueNetId);
}

void UGunnerSessionHelperSubsystem::OnSessionParticipantLeft(FName SessionName, const FUniqueNetId& UniqueNetId, EOnSessionParticipantLeftReason OnSessionParticipantLeftReason)
{
	Online::GetSubsystem(GetWorld())->GetSessionInterface()->ClearOnSessionParticipantLeftDelegate_Handle(OnSessionParticipantLeftDelegateHandle);
	OnSessionParticipantLeftDelegateMulticast.Broadcast(SessionName, UniqueNetId, ToLogString(OnSessionParticipantLeftReason));
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
