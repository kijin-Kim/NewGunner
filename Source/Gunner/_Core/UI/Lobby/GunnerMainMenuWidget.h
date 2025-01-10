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
	virtual void NativeConstruct() override;

private:
	void OnParticipantsChanged(FName SessionName, const FUniqueNetId& UniqueNetId, bool bJoined);

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName Name, EOnJoinSessionCompleteResult::Type Arg);

	UFUNCTION(BlueprintCallable)
	void OnHostButtonClicked(FString RoomName, FString MapName);
	UFUNCTION(BlueprintCallable)
	void OnShutdownButtonClicked();

	UFUNCTION(BlueprintCallable)
	void StartGame();

	UFUNCTION(BlueprintCallable)
	void JoinSession(FString SessionId);
	UFUNCTION(BlueprintCallable)
	void FindSession(FString RoomName);

	UFUNCTION(BlueprintCallable)
	bool IsLocalPlayerHost() const;
	
	FString GetPlayerNickname(const FUniqueNetId& UserId) const;
	TArray<FString> GetParticipants(FNamedOnlineSession* Session) const;

	

public:
	UPROPERTY(BlueprintAssignable)
	FOnSessionFindCompletedSignature OnSessionFindCompleted;
	UPROPERTY(BlueprintAssignable)
	FOnJoinSessionLobbySucceededSignature OnJoinSessionLobbySucceeded;
	UPROPERTY(BlueprintAssignable)
	FOnJoinedSessionParticipantsChangedSignature OnJoinedSessionParticipantsChanged;

private:
	TSharedPtr<FOnlineSessionSearch> SessionSearch;


	IOnlineSessionPtr SessionInterfacePtr;

	FDelegateHandle OnCreateSessionCompleteDelegateHandle;
	FDelegateHandle OnStartSessionCompleteDelegateHandle;
	FDelegateHandle OnFindSessionsCompleteDelegateHandle;
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;
	FDelegateHandle OnDestroySessionCompleteDelegateHandle;
};
