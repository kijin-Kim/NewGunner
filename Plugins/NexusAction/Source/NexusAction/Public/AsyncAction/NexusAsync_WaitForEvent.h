// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NexusAsync.h"
#include "Event/NexusEventInterface.h"
#include "NexusAsync_WaitForEvent.generated.h"

class UNexusAction;
class UNexusEventManagerComponent;
/**
 * 로컬에서 이벤트를 기다립니다. 네트워킹 이벤트를 원하면 SendOrWaitEvent/WaitForSync를 사용하세요.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNexusEventReceivedSignature, FGameplayTag, EventTag);


UCLASS()
class NEXUSACTION_API UNexusAsync_WaitForEvent : public UNexusAsync, public INexusEventInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (HidePin = "InAction", DefaultToSelf = "InAction", BlueprintInternalUseOnly = "true"))
	static UNexusAsync_WaitForEvent* WaitForEvent(UNexusAction* InAction, AActor* EventTargetActor, FGameplayTag EventTag, UScriptStruct* EventMessageType, bool bInAutoCancel = true);
	
	virtual void Activate() override;
	virtual void SetReadyToDestroy() override;

	virtual TArray<FNexusEventCallbackHandle> SetupEvents() override;


	UFUNCTION(BlueprintCallable, CustomThunk, meta = (CustomStructureParam = "OutMessage"))
	bool GetMessage(UPARAM(ref) int32& OutMessage);
	DECLARE_FUNCTION(execGetMessage);

public:
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnEventReceived"))
	FOnNexusEventReceivedSignature OnEventReceivedDelegate;

private:
	const void* MessagePtr;
	TWeakObjectPtr<UNexusEventManagerComponent> TargetEventManagerComponent;
	TWeakObjectPtr<UScriptStruct> EventMesageType;
	FGameplayTag EventTag;
	bool bAutoCancel = true;

};
