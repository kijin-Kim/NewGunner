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
	int32 PingInMs;

	FString ToString() const
	{
		return FString::Printf(TEXT("RoomName: %s, MapName: %s, PingInMs: %d"), *RoomName, *MapName, PingInMs);
	}
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionFindCompletedSignature, const TArray<FRoomInfo>&, RoomInfos);

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
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName Name, EOnJoinSessionCompleteResult::Type Arg);

	UFUNCTION(BlueprintCallable)
	void OnHostButtonClicked(FString RoomName, FString MapName);
	UFUNCTION(BlueprintCallable)
	void OnShutdownButtonClicked();

	void JoinSession();
	UFUNCTION(BlueprintCallable)
	void FindSessions();

public:
	UPROPERTY(BlueprintAssignable)
	FOnSessionFindCompletedSignature OnSessionFindCompleted;

private:
	TSharedPtr<FOnlineSessionSearch> SessionSearch;


	IOnlineSessionPtr SessionInterfacePtr;

	FDelegateHandle OnCreateSessionCompleteDelegateHandle;
	FDelegateHandle OnStartSessionCompleteDelegateHandle;
	FDelegateHandle OnFindSessionsCompleteDelegateHandle;
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;
	FDelegateHandle OnDestroySessionCompleteDelegateHandle;
};
