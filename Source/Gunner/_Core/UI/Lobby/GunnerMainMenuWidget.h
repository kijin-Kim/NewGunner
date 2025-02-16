// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystem.h"
#include "Gunner/_Core/UI/GunnerUserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "GunnerMainMenuWidget.generated.h"

class UWidgetSwitcher;
class UGunnerButtonWidget;
class FOnlineSessionSearch;

USTRUCT(BlueprintType)
struct FRoomInfo
{
	GENERATED_BODY()

	FRoomInfo()
		: RoomName(TEXT("None"))
		  , MapName(TEXT("None"))
		  , PlayerCount(0)
		  , MaxPlayerCount(0)
		  , PingInMs(0)
		  , SessionId(TEXT("None"))
	{
	}

	UPROPERTY(BlueprintReadOnly)
	FString RoomName;
	UPROPERTY(BlueprintReadOnly)
	FString MapName;
	UPROPERTY(BlueprintReadOnly)
	int32 PlayerCount;
	UPROPERTY(BlueprintReadOnly)
	int32 MaxPlayerCount;
	UPROPERTY(BlueprintReadOnly)
	int32 PingInMs;
	UPROPERTY(BlueprintReadOnly)
	FString SessionId;
	UPROPERTY(BlueprintReadOnly)
	TArray<FString> Participants;

	FString ToString() const
	{
		FString ParticipantsString;
		for (const FString& Participant : Participants)
		{
			ParticipantsString += Participant + TEXT(", ");
		}
		return FString::Printf(TEXT("RoomName: %s, MapName: %s, PlayerCount: %d, MaxPlayerCount: %d, PingInMs: %d, SessionId: %s, Participants: %s"), *RoomName, *MapName, PlayerCount, MaxPlayerCount, PingInMs, *SessionId, *ParticipantsString);
	}
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionFindCompletedSignature, const TArray<FRoomInfo>&, RoomInfos);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJoinSessionLobbySucceededSignature, const FRoomInfo&, RoomInfo);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJoinedSessionParticipantsChangedSignature, const TArray<FString>&, Participants);

/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerMainMenuWidget : public UGunnerUserWidget
{
	GENERATED_BODY()

public:
	UGunnerMainMenuWidget();

private:
	void OnSessionParticipantJoined(FName Name, const FUniqueNetId& UniqueNetId);
	void OnSessionParticipantLeft(FName Name, const FUniqueNetId& UniqueNetId, EOnSessionParticipantLeftReason OnSessionParticipantLeftReason);

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName Name, EOnJoinSessionCompleteResult::Type Arg);
	void OnStartSessionComplete(FName Name, bool bArg);


	UFUNCTION(BlueprintCallable)
	void OnHostButtonClicked(FString RoomName, FString MapName);
	UFUNCTION(BlueprintCallable)
	void OnShutdownButtonClicked();

	UFUNCTION(BlueprintCallable)
	void StartGame();
	UFUNCTION(BlueprintCallable)
	bool CanStartGame() const;

	UFUNCTION(BlueprintCallable)
	void JoinSession(FString SessionId);
	UFUNCTION(BlueprintCallable)
	void FindSession(FString RoomName);
	UFUNCTION(BlueprintCallable)
	void LeaveSession();


	UFUNCTION(BlueprintCallable)
	bool IsLocalPlayerHost() const;

	FString GetPlayerNickname(const FUniqueNetId& UserId) const;
	TArray<FString> GetParticipants(FNamedOnlineSession* Session) const;
	FString DecodeString(const FString& TargetString) const;

public:
	UPROPERTY(BlueprintAssignable)
	FOnSessionFindCompletedSignature OnSessionFindCompleted;
	UPROPERTY(BlueprintAssignable)
	FOnJoinSessionLobbySucceededSignature OnJoinSessionLobbySucceeded;
	UPROPERTY(BlueprintAssignable)
	FOnJoinedSessionParticipantsChangedSignature OnJoinedSessionParticipantsChanged;

private:
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	FDelegateHandle OnSessionParticipantJoinedDelegateHandle;
	FDelegateHandle OnSessionParticipantLeftDelegateHandle;
	FDelegateHandle OnCreateSessionCompleteDelegateHandle;
	FDelegateHandle OnFindSessionsCompleteDelegateHandle;
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;
	FDelegateHandle OnStartSessionCompleteDelegateHandle;


	// Delegates
	FOnSessionParticipantJoinedDelegate OnSessionParticipantJoinedDelegate;
	FOnSessionParticipantLeftDelegate OnSessionParticipantLeftDelegate;
	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;
	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;
	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;
};
