// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerOnlineSessionCancellableAsyncAction.h"
#include "GunnerSessionHelperSubsystem.h"
#include "GunnerSessionAsync_FindSession.generated.h"

USTRUCT()
struct FGunnerSessionLobbyInfo
{
	GENERATED_BODY()
	
	int32 PingInMs;
	FString LobbyName;
	FString MapName;
	int32 CurrentPlayerCount;
	int32 MaxPlayerCount;
};


/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerSessionAsync_FindSession : public UGunnerOnlineSessionCancellableAsyncAction
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Gunner|OnlineSession", meta = (WorldContext = "InWorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Find Session"))
	static UGunnerSessionAsync_FindSession* FindSession(UObject* InWorldContextObject, FString InLobbyName);
	virtual void Activate() override;

private:
	UFUNCTION()
	void OnFindSessionComplete(bool bWasSuccessful);

public:
	UPROPERTY(BlueprintAssignable)
	FOnFindSessionsCompleteSignature OnCompleted;

private:
	TWeakObjectPtr<UObject> WorldContextObject;
	FString LobbyName;
};
