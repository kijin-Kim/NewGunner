// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintDataDefinitions.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "GunnerSessionHelperSubsystem.generated.h"

#define DECLARE_DELEGATE_AND_HANDLE(DelegateType, DelegateName) \
DelegateType DelegateName; \
FDelegateHandle DelegateName##Handle;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSessionParticipantJoinedSignature, FName, SessionName, const FUniqueNetIdRepl&, UniqueNetId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSessionParticipantLeftSignature, FName, SessionName, const FUniqueNetIdRepl&, UniqueNetId, FString, OnSessionParticipantLeftReason);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCreateSessionCompleteSignature, FName, SessionName, bool, bWasSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFindSessionsCompleteSignature, bool, bWasSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnJoinSessionCompleteSignature, FName, SessionName, FString, JoinSessionCompleteResult);


/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerSessionHelperSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UGunnerSessionHelperSubsystem();
	void CreateSession(FString LobbyName, FString MapName, int32 MaxNumPlayers);
	void FindLobbies(FString LobbyName = TEXT("None"));
	void JoinSession();
	void LeaveSession();


	IOnlineSessionPtr GetSessionInterface() const;

private:
	void OnSessionParticipantJoined(FName Name, const FUniqueNetId& UniqueNetId);
	void OnSessionParticipantLeft(FName SessionName, const FUniqueNetId& UniqueNetId, EOnSessionParticipantLeftReason OnSessionParticipantLeftReason);
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

public:
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnSessionParticipantJoined"))
	FOnSessionParticipantJoinedSignature OnSessionParticipantJoinedDelegateMulticast;

	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnSessionParticipantLeft"))
	FOnSessionParticipantLeftSignature OnSessionParticipantLeftDelegateMulticast;

	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnCreateSessionComplete"))
	FOnCreateSessionCompleteSignature OnCreateSessionCompleteDelegateMulticast;

	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnFindSessionsComplete"))
	FOnFindSessionsCompleteSignature OnFindSessionsCompleteDelegateMulticast;

	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnJoinSessionComplete"))
	FOnJoinSessionCompleteSignature OnJoinSessionCompleteDelegateMulticast;

private:
	TSharedPtr<FOnlineSessionSearch> OnlineSessionSearch;
	DECLARE_DELEGATE_AND_HANDLE(FOnSessionParticipantJoinedDelegate, OnSessionParticipantJoinedDelegate);
	DECLARE_DELEGATE_AND_HANDLE(FOnSessionParticipantLeftDelegate, OnSessionParticipantLeftDelegate);
	DECLARE_DELEGATE_AND_HANDLE(FOnCreateSessionCompleteDelegate, OnCreateSessionCompleteDelegate);
	DECLARE_DELEGATE_AND_HANDLE(FOnFindSessionsCompleteDelegate, OnFindSessionsCompleteDelegate);
	DECLARE_DELEGATE_AND_HANDLE(FOnJoinSessionCompleteDelegate, OnJoinSessionCompleteDelegate);
};
