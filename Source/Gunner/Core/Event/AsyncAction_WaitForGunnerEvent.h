// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EventCallbackBindInterface.h"
#include "GameplayTagContainer.h"
#include "Engine/CancellableAsyncAction.h"
#include "AsyncAction_WaitForGunnerEvent.generated.h"

class UEventManagerComponent;
/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGunnerEventReceivedSignature, FGameplayTag, EventTag);


UCLASS()
class GUNNER_API UAsyncAction_WaitForGunnerEvent : public UCancellableAsyncAction, public IEventCallbackBindInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UAsyncAction_WaitForGunnerEvent* WaitForGunnerEvent(AActor* EventTargetActor, FGameplayTag EventTag, UScriptStruct* EventMessageType);
	virtual void Activate() override;
	virtual void SetReadyToDestroy() override;
	
	virtual TArray<FEventCallbackHandle> SetupEvents() override;
	virtual UEventManagerComponent* GetEventManagerComponent() const override;
	
	
	UFUNCTION(BlueprintCallable, CustomThunk, meta = (CustomStructureParam = "OutMessage"))
	bool GetMessage(UPARAM(ref) int32& OutMessage);

	DECLARE_FUNCTION(execGetMessage);

public:
	UPROPERTY(BlueprintAssignable)
	FOnGunnerEventReceivedSignature OnEventReceived;

private:
	const void* MessagePtr;
	TWeakObjectPtr<UEventManagerComponent> TargetEventManagerComponent;
	TWeakObjectPtr<UScriptStruct> EventMesageType;
	FGameplayTag EventTag;
};
