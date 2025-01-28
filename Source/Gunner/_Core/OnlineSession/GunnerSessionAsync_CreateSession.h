// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerOnlineSessionCancellableAsyncAction.h"
#include "GunnerSessionHelperSubsystem.h"
#include "GunnerSessionAsync_CreateSession.generated.h"

/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerSessionAsync_CreateSession : public UGunnerOnlineSessionCancellableAsyncAction
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Gunner|OnlineSession", meta = (WorldContext = "InWorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Create Session"))
	static UGunnerSessionAsync_CreateSession* CreateSession(UObject* InWorldContextObject, FString InLobbyName, FString InMapName, int32 InMaxNumPlayers);

	virtual void Activate() override;

private:
	UFUNCTION()
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

public:
	UPROPERTY(BlueprintAssignable)
	FOnCreateSessionCompleteSignature OnCompleted;

private:
	TWeakObjectPtr<UObject> WorldContextObject;
	FString LobbyName;
	FString MapName;
	int32 MaxNumPlayers;
};
