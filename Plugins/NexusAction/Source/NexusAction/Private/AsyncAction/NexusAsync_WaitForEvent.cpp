// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncAction/NexusAsync_WaitForEvent.h"

#include "Event/NexusEventManagerComponent.h"

UNexusAsync_WaitForEvent* UNexusAsync_WaitForEvent::WaitForEvent(UNexusAction* InAction, AActor* EventTargetActor, FGameplayTag InEventTag, UScriptStruct* InEventMessageType, bool bInAutoCancel)
{
	UNexusAsync_WaitForEvent* SelfObject = NewNexusAsync<UNexusAsync_WaitForEvent>(InAction);
	if (!EventTargetActor || !InEventTag.IsValid() || !InEventMessageType)
	{
		return nullptr;
	}

	SelfObject->TargetEventManagerComponent = UNexusEventManagerComponent::GetEventManagerComponentFromActor(EventTargetActor);
	if (!SelfObject->TargetEventManagerComponent.IsValid())
	{
		return nullptr;
	}

	SelfObject->EventTag = InEventTag;
	SelfObject->EventMesageType = InEventMessageType;
	SelfObject->bAutoCancel = bInAutoCancel;
	SelfObject->RegisterWithGameInstance(EventTargetActor->GetWorld());

	return SelfObject;
}

void UNexusAsync_WaitForEvent::Activate()
{
	Super::Activate();
	UnbindEvents(TargetEventManagerComponent.Get());
	BindEvents();
}

void UNexusAsync_WaitForEvent::SetReadyToDestroy()
{
	Super::SetReadyToDestroy();
	MessagePtr = nullptr;
	UnbindEvents(TargetEventManagerComponent.Get());
}

TArray<FNexusEventCallbackHandle> UNexusAsync_WaitForEvent::SetupEvents()
{
	TWeakObjectPtr<UNexusAsync_WaitForEvent> Weak = this;
	return {
		TargetEventManagerComponent->BindEventCallbackInternal(EventTag, [Weak](FGameplayTag Tag, const void* MessagePtr)
		{
			UNexusAsync_WaitForEvent* Strong = Weak.Get();
			if (Strong && Strong->ShouldBroadcastDelegates())
			{
				Strong->MessagePtr = MessagePtr;
				Strong->OnEventReceivedDelegate.Broadcast(Tag);
				if (Strong->bAutoCancel)
				{
					Strong->Cancel();
				}
			}
		}, EventMesageType.Get())
	};
}

DEFINE_FUNCTION(UNexusAsync_WaitForEvent::execGetMessage)
{
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	void* OutMessagePtr = Stack.MostRecentPropertyAddress;
	FStructProperty* StructProperty = CastField<FStructProperty>(Stack.MostRecentProperty);
	P_FINISH;

	if (!P_THIS->MessagePtr)
	{
		*StaticCast<bool*>(RESULT_PARAM) = false;
		return;
	}

	if (!StructProperty || !StructProperty->Struct || !OutMessagePtr)
	{
		*StaticCast<bool*>(RESULT_PARAM) = false;
		return;
	}

	if (StructProperty->Struct != P_THIS->EventMesageType.Get())
	{
		*StaticCast<bool*>(RESULT_PARAM) = false;
		return;
	}

	StructProperty->Struct->CopyScriptStruct(OutMessagePtr, P_THIS->MessagePtr);
	*StaticCast<bool*>(RESULT_PARAM) = true;
}
