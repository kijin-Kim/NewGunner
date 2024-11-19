// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerCancellableAsyncAction.h"
#include "Delegates/Delegate.h"
#include "GunnerActionAsync_WaitForSync.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGunnerActionNetSyncSignature);
/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerActionAsync_WaitForSync : public UGunnerCancellableAsyncAction
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (HidePin = "InAction", DefaultToSelf = "InAction", BlueprintInternalUseOnly = "true"))
	static UGunnerActionAsync_WaitForSync* WaitForSync(UGunnerAction* InAction);
	
	//~ Begin UCancellableAsyncAction Interface.
	virtual void Activate() override;
	//~ End UCancellableAsyncAction Interface.
	
	void OnSync();

public:
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnSync"))
	FOnGunnerActionNetSyncSignature OnSyncDelegate;
};
