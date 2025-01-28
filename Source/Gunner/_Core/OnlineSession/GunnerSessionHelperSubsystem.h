// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "GunnerSessionHelperSubsystem.generated.h"

#define DECLARE_DELEGATE_AND_HANDLE(DelegateType, DelegateName) \
DelegateType DelegateName; \
FDelegateHandle DelegateName##Handle;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRegisterPlayersCompleteSignature, FName, SessionName, const TArray<FUniqueNetIdRepl>&, Players, bool, bWasSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnUnregisterPlayersCompleteSignature, FName, SessionName, const TArray<FUniqueNetIdRepl>&, Players, bool, bWasSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCreateSessionCompleteSignature, FName, SessionName, bool, bWasSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFindSessionsCompleteSignature, bool, bWasSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnJoinSessionCompleteSignature, FName, SessionName, FString, JoinSessionCompleteResult);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDestroySessionCompleteSignature, FName, SessionName, bool, bWasSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSessionFailureSignature, const FUniqueNetIdRepl&, UniqueNetId, FString, SessionFailure);


/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerSessionHelperSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UGunnerSessionHelperSubsystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	UFUNCTION(BlueprintCallable)
	void CreateSession(FString LobbyName, FString MapName, int32 MaxNumPlayers);
	UFUNCTION(BlueprintCallable)
	void FindSessions(FString LobbyName = TEXT("None"));
	UFUNCTION(BlueprintCallable)
	void JoinSession(int32 SessionResultIndex);
	UFUNCTION(BlueprintCallable)
	void DestroySession();
	UFUNCTION(BlueprintCallable)
	void CancelFindSessions();

	UFUNCTION(BlueprintCallable)
	void EnableTraceSessionFailure(bool bEnable);
	UFUNCTION(BlueprintCallable)
	void EnableTraceRegisteredPlayers(bool bEnable);

	IOnlineSessionPtr GetSessionInterface() const;
	IOnlineIdentityPtr GetIdentityInterface() const;
	TSharedPtr<FOnlineSessionSearch> GetOnlineSessionSearch() const { return OnlineSessionSearch; }

private:
	void OnRegisterPlayersComplete(FName SessionName, const TArray<FUniqueNetIdRef>& Players, bool bWasSuccessful);
	void OnUnregisterPlayersComplete(FName SessionName, const TArray<FUniqueNetIdRef>& Players, bool bWasSuccessful);
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnSessionFailure(const FUniqueNetId& UniqueNetId, ESessionFailure::Type FailureType);
	
	void OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& FailureString);

public:
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnRegisterPlayersComplete"))
	FOnRegisterPlayersCompleteSignature OnRegisterPlayersCompleteDelegateMulticast;

	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnUnregisterPlayersComplete"))
	FOnUnregisterPlayersCompleteSignature OnUnregisterPlayersCompleteDelegateMulticast;

	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnCreateSessionComplete"))
	FOnCreateSessionCompleteSignature OnCreateSessionCompleteDelegateMulticast;

	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnFindSessionsComplete"))
	FOnFindSessionsCompleteSignature OnFindSessionsCompleteDelegateMulticast;

	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnJoinSessionComplete"))
	FOnJoinSessionCompleteSignature OnJoinSessionCompleteDelegateMulticast;

	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnDestroySessionComplete"))
	FOnDestroySessionCompleteSignature OnDestroySessionCompleteDelegateMulticast;

	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnSessionFailure"))
	FOnSessionFailureSignature OnSessionFailureDelegateMulticast;

private:
	TSharedPtr<FOnlineSessionSearch> OnlineSessionSearch;
	
	DECLARE_DELEGATE_AND_HANDLE(FOnRegisterPlayersCompleteDelegate, OnRegisterPlayersCompleteDelegate);
	DECLARE_DELEGATE_AND_HANDLE(FOnUnregisterPlayersCompleteDelegate, OnUnregisterPlayersCompleteDelegate);
	DECLARE_DELEGATE_AND_HANDLE(FOnCreateSessionCompleteDelegate, OnCreateSessionCompleteDelegate);
	DECLARE_DELEGATE_AND_HANDLE(FOnFindSessionsCompleteDelegate, OnFindSessionsCompleteDelegate);
	DECLARE_DELEGATE_AND_HANDLE(FOnJoinSessionCompleteDelegate, OnJoinSessionCompleteDelegate);
	DECLARE_DELEGATE_AND_HANDLE(FOnDestroySessionCompleteDelegate, OnDestroySessionCompleteDelegate);
	DECLARE_DELEGATE_AND_HANDLE(FOnSessionFailureDelegate, OnSessionFailureDelegate);
};
