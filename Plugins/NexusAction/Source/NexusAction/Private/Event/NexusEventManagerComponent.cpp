// Fill out your copyright notice in the Description page of Project Settings.


#include "Event/NexusEventManagerComponent.h"

#include "NexusLog.h"
#include "Event/NexusEventMangerInterface.h"


UNexusEventManagerComponent::UNexusEventManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UNexusEventManagerComponent::UnbindEventCallback(FNexusEventCallbackHandle Handle)
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

void UNexusEventManagerComponent::UnbindAllEventCallbacks()
{
	for (auto& EventCallback : EventCallbacks)
	{
		FNexusEventCallbackListScopeLock CallbackListScopeLock(EventCallback.Value);
		for (const auto& Callback : EventCallback.Value.Callbacks)
		{
			UnbindEventCallback({Callback.HandleID, EventCallback.Key});
		}
	}
}

void UNexusEventManagerComponent::HandleEvent(FGameplayTag EventTag, const void* Message, UScriptStruct* MessageType)
{
	if (EventCallbacks.Contains(EventTag))
	{
		FNexusEventCallbackListScopeLock CallbackListScopeLock(EventCallbacks[EventTag]);
		for (const auto& Callback : EventCallbacks[EventTag].Callbacks)
		{
			NX_VLOG_SUB(GetOwner(), LogNexus, Verbose, TEXT("이벤트 [%s] 발생"), *EventTag.ToString());
			Callback(EventTag, Message, MessageType);
		}
	}
}

UNexusEventManagerComponent* UNexusEventManagerComponent::GetEventManagerComponentFromActor(AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}

	if (INexusEventManagerInterface* EventManagerInterface = Cast<INexusEventManagerInterface>(Actor))
	{
		return EventManagerInterface->GetEventManagerComponent();
	}

	if (UNexusEventManagerComponent* EventManagerComponent = Actor->GetComponentByClass<UNexusEventManagerComponent>())
	{
		return EventManagerComponent;
	}

	return nullptr;
}

FNexusEventCallbackHandle UNexusEventManagerComponent::BindEventCallbackInternal(FGameplayTag EventTag, TFunction<void(FGameplayTag, const void*)>&& Callbacks, UScriptStruct* MessageType)
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
		return FNexusEventCallbackHandle(EventCallbacks[EventTag].HandleID, EventTag);
	}

	EventCallbackList.Callbacks.Add({EventCallbackList.HandleID, MoveTemp(Callbacks), MessageType});
	return FNexusEventCallbackHandle(EventCallbacks[EventTag].HandleID, EventTag);
}


DEFINE_FUNCTION(UNexusEventManagerComponent::execBP_SendEventToActor)
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

	if (UNexusEventManagerComponent* EventManagerComponent = GetEventManagerComponentFromActor(TargetActor))
	{
		EventManagerComponent->HandleEvent(EventTag, MessagePtr, StructProperty->Struct);
	}
}


void UNexusEventManagerComponent::FEventCallback::operator()(FGameplayTag EventTag, const void* MessagePtr, UScriptStruct* InMessageType) const
{
	if (MessageType != InMessageType)
	{
		UE_LOG(LogNexus, Error, TEXT("Message type mismatch"));
		return;
	}
	Callback(EventTag, MessagePtr);
}

void UNexusEventManagerComponent::FEventCallbackList::IncrementCallbackListLock()
{
	CallbackListScopeLockCount++;
}

void UNexusEventManagerComponent::FEventCallbackList::DecrementCallbackListLock()
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
