// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GunnerCancellableAsyncAction.h"
#include "Gunner/_Core/Event/GunnerEventCallbackBindInterface.h"
#include "GunnerActionAsync_WaitForGunnerEvent.generated.h"

class UGunnerAction;
class UGunnerEventManagerComponent;
/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGunnerEventReceivedSignature, FGameplayTag, EventTag);


UCLASS()
class GUNNER_API UGunnerActionAsync_WaitForGunnerEvent : public UGunnerCancellableAsyncAction, public IGunnerEventCallbackBindInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (HidePin = "InAction", DefaultToSelf = "InAction", BlueprintInternalUseOnly = "true"))
	static UGunnerActionAsync_WaitForGunnerEvent* WaitForGunnerEvent(UGunnerAction* InAction, AActor* EventTargetActor, FGameplayTag EventTag, UScriptStruct* EventMessageType);
	
	//~ Begin UCancellableAsyncAction Interface.
	virtual void Activate() override;
	virtual void SetReadyToDestroy() override;
	//~ End UCancellableAsyncAction Interface.

	//~ Begin IGunnerEventCallbackBindInterface Interface.
	virtual TArray<FGunnerEventCallbackHandle> SetupEvents() override;
	//~ Begin IGunnerEventCallbackBindInterface Interface.


	UFUNCTION(BlueprintCallable, CustomThunk, meta = (CustomStructureParam = "OutMessage"))
	bool GetMessage(UPARAM(ref) int32& OutMessage);
	DECLARE_FUNCTION(execGetMessage);

public:
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnEventReceived"))
	FOnGunnerEventReceivedSignature OnEventReceivedDelegate;

private:
	const void* MessagePtr;
	TWeakObjectPtr<UGunnerEventManagerComponent> TargetEventManagerComponent;
	TWeakObjectPtr<UScriptStruct> EventMesageType;
	FGameplayTag EventTag;
};
