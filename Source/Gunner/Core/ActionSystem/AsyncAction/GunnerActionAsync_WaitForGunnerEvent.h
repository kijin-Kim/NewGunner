// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gunner/Core/Event/EventCallbackBindInterface.h"
#include "GameplayTagContainer.h"
#include "GunnerCancellableAsyncAction.h"
#include "GunnerActionAsync_WaitForGunnerEvent.generated.h"

class UGunnerAction;
class UEventManagerComponent;
/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGunnerEventReceivedSignature, FGameplayTag, EventTag);


UCLASS()
class GUNNER_API UGunnerActionAsync_WaitForGunnerEvent : public UGunnerCancellableAsyncAction, public IEventCallbackBindInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (HidePin = "InAction", DefaultToSelf = "InAction", BlueprintInternalUseOnly = "true"))
	static UGunnerActionAsync_WaitForGunnerEvent* WaitForGunnerEvent(UGunnerAction* InAction, AActor* EventTargetActor, FGameplayTag EventTag, UScriptStruct* EventMessageType);
	virtual void Activate() override;
	virtual void SetReadyToDestroy() override;
	
	virtual TArray<FEventCallbackHandle> SetupEvents() override;
	virtual UEventManagerComponent* GetEventManagerComponent() const override;
	
	
	UFUNCTION(BlueprintCallable, CustomThunk, meta = (CustomStructureParam = "OutMessage"))
	bool GetMessage(UPARAM(ref) int32& OutMessage);

	DECLARE_FUNCTION(execGetMessage);

public:
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnEventReceived"))
	FOnGunnerEventReceivedSignature OnEventReceivedDelegate;

private:
	const void* MessagePtr;
	TWeakObjectPtr<UEventManagerComponent> TargetEventManagerComponent;
	TWeakObjectPtr<UScriptStruct> EventMesageType;
	FGameplayTag EventTag;
};
