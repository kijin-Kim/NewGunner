// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerOnlineSessionCancellableAsyncAction.h"
#include "GunnerSessionHelperSubsystem.h"
#include "GunnerSessionAsync_DestroySession.generated.h"

/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerSessionAsync_DestroySession : public UGunnerOnlineSessionCancellableAsyncAction
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Gunner|OnlineSession", meta = (WorldContext = "InWorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Destroy Session"))
	static UGunnerSessionAsync_DestroySession* DestroySession(UObject* InWorldContextObject);

	
	virtual void Activate() override;

private:
	UFUNCTION()
	void OnDestroySessionComplete(FName InSessionName, bool bWasSuccessful);
	

public:
	UPROPERTY(BlueprintAssignable)
	FOnDestroySessionCompleteSignature OnCompleted;

private:
	TWeakObjectPtr<UObject> WorldContextObject;
};
