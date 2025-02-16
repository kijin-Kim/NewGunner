// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NexusAsync.h"
#include "Delegates/Delegate.h"
#include "NexusAsync_WaitForSync.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNexusSyncSignature);
/**
 * 
 */
UCLASS()
class NEXUSACTION_API UNexusAsync_WaitForSync : public UNexusAsync
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (HidePin = "InAction", DefaultToSelf = "InAction", BlueprintInternalUseOnly = "true"))
	static UNexusAsync_WaitForSync* WaitForSync(UNexusAction* InAction);
	
	//~ Begin UCancellableAsyncAction Interface.
	virtual void Activate() override;
	//~ End UCancellableAsyncAction Interface.
	
	void OnSync();

public:
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnSync"))
	FOnNexusSyncSignature OnSyncDelegate;
};
