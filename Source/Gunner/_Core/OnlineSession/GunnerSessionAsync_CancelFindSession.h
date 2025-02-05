// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerOnlineSessionCancellableAsyncAction.h"
#include "GunnerSessionHelperSubsystem.h"
#include "GunnerSessionAsync_CancelFindSession.generated.h"


/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerSessionAsync_CancelFindSession : public UGunnerOnlineSessionCancellableAsyncAction
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Gunner|OnlineSession", meta = (WorldContext = "InWorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Cancel Find Session"))
	static UGunnerSessionAsync_CancelFindSession* CancelFindSession(UObject* InWorldContextObject);
	virtual void Activate() override;

private:
	UFUNCTION()
	void OnCancelFindSessionComplete(bool bWasSuccessful);

public:
	TWeakObjectPtr<UObject> WorldContextObject;
	UPROPERTY(BlueprintAssignable)
	FOnCancelFindSessionsCompleteSignature OnCompleted;
};
