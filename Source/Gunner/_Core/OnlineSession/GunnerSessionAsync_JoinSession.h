// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerOnlineSessionCancellableAsyncAction.h"
#include "GunnerSessionHelperSubsystem.h"
#include "GunnerSessionAsync_JoinSession.generated.h"

/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerSessionAsync_JoinSession : public UGunnerOnlineSessionCancellableAsyncAction
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Gunner|OnlineSession", meta = (WorldContext = "InWorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Join Session"))
	static UGunnerSessionAsync_JoinSession* JoinSession(UObject* InWorldContextObject, APlayerController* InPlayerController, FString InSessionIdStr);

	virtual void Activate() override;

private:
	UFUNCTION()
	void OnJoinSessionComplete(FName SessionName, FString JoinSessionCompleteResult);

public:
	UPROPERTY(BlueprintAssignable)
	FOnJoinSessionCompleteSignature OnCompleted;

private:
	TWeakObjectPtr<UObject> WorldContextObject;
	TWeakObjectPtr<APlayerController> PlayerController;
	FString SeesionIdStr;
};
