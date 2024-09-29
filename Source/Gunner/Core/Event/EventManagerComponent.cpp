// Fill out your copyright notice in the Description page of Project Settings.


#include "EventManagerComponent.h"

#include "Gunner/Gunner.h"


UEventManagerComponent::UEventManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEventManagerComponent::UnbindEventCallback(FEventCallbackHandle Handle)
{
	if (!Handle.IsValid() || !EventCallbacks.Contains(Handle.EventTag))
	{
		return;
	}

	if (EventCallbacks[Handle.EventTag].CallbackListScopeLockCount > 0)
	{
		EventCallbacks[Handle.EventTag].CallbackPendingRemoves.Add(Handle);
		return;
	}

	EventCallbacks[Handle.EventTag].Callbacks.RemoveAll([Handle](const FEventCallback& Callback)
	{
		return Callback.HandleID == Handle.ID;
	});
}

void UEventManagerComponent::HandleEvent(FGameplayTag EventTag, const void* Message, UScriptStruct* MessageType)
{
	if (EventCallbacks.Contains(EventTag))
	{
		FGunnerEventCallbackListScopeLock CallbackListScopeLock(EventCallbacks[EventTag]);
		for (const auto& Callback : EventCallbacks[EventTag].Callbacks)
		{
			Callback(EventTag, Message, MessageType);
		}
	}
}

FEventCallbackHandle UEventManagerComponent::BindEventCallbackInternal(FGameplayTag EventTag, TFunction<void(FGameplayTag, const void*)>&& Callbacks, UScriptStruct* MessageType)
{
	if (!EventTag.IsValid())
	{
		return {};
	}

	auto& EventCallbackList = EventCallbacks.FindOrAdd(EventTag);
	EventCallbackList.HandleID++;

	if (EventCallbackList.CallbackListScopeLockCount > 0)
	{
		EventCallbackList.CallbackPendingAdds.Add({EventCallbackList.HandleID, MoveTemp(Callbacks), MessageType});
		return FEventCallbackHandle(EventCallbacks[EventTag].HandleID, EventTag);
	}

	EventCallbackList.Callbacks.Add({EventCallbackList.HandleID, MoveTemp(Callbacks), MessageType});
	return FEventCallbackHandle(EventCallbacks[EventTag].HandleID, EventTag);
}


DEFINE_FUNCTION(UEventManagerComponent::execBP_SendEventToActor)
{
	P_GET_STRUCT(FGameplayTag, EventTag);
	P_GET_OBJECT(AActor, TargetActor);
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	FStructProperty* StructProperty = CastField<FStructProperty>(Stack.MostRecentProperty);
	void* MessagePtr = Stack.MostRecentPropertyAddress;


	P_FINISH;

	if (!EventTag.IsValid() || !TargetActor || !MessagePtr)
	{
		return;
	}

	if (UEventManagerComponent* EventManagerComponent = TargetActor->GetComponentByClass<UEventManagerComponent>())
	{
		EventManagerComponent->HandleEvent(EventTag, MessagePtr, StructProperty->Struct);
	}
}


void UEventManagerComponent::FEventCallback::operator()(FGameplayTag EventTag, const void* MessagePtr, UScriptStruct* InMessageType) const
{
	if (MessageType != InMessageType)
	{
		UE_LOG(LogGunner, Error, TEXT("Message type mismatch"));
		return;
	}
	Callback(EventTag, MessagePtr);
}

void UEventManagerComponent::FEventCallbackList::IncrementCallbackListLock()
{
	CallbackListScopeLockCount++;
}

void UEventManagerComponent::FEventCallbackList::DecrementCallbackListLock()
{
	CallbackListScopeLockCount--;
	if (CallbackListScopeLockCount == 0 && (CallbackPendingAdds.IsEmpty() || CallbackPendingRemoves.IsEmpty()))
	{
		Callbacks.Append(CallbackPendingAdds);
		CallbackPendingAdds.Empty();

		for (const auto& Handle : CallbackPendingRemoves)
		{
			Callbacks.RemoveAll([Handle](const FEventCallback& Callback)
			{
				return Callback.HandleID == Handle.ID;
			});
		}

		
		
		CallbackPendingRemoves.Empty();
	}
}
