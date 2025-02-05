// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerOnlineSessionCancellableAsyncAction.h"
#include "GunnerSessionHelperSubsystem.h"
#include "GunnerSessionAsync_FindSession.generated.h"


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
	virtual void Cancel() override;

private:
	UFUNCTION()
	void OnFindSessionComplete(bool bWasSuccessful, const TArray<FGunnerSessionLobbyInfo>& LobbyInfos);


public:
	UPROPERTY(BlueprintAssignable)
	FOnFindSessionsCompleteSignature OnCompleted;

private:
	TWeakObjectPtr<UObject> WorldContextObject;
	FString LobbyName;
};
