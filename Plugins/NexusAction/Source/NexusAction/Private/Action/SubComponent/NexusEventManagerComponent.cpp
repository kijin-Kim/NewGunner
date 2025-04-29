// Fill out your copyright notice in the Description page of Project Settings.


#include "Action/SubComponent/NexusEventManagerComponent.h"

#include "NexusLog.h"


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
			if (Callback.MessageType != MessageType)
			{
				NX_VLOG_SUB(GetAgentActor(), LogNexus, Error, TEXT("메시지 타입 불일치: ExpectedMessageType=%s, MessageType=%s"), *Callback.MessageType->GetName(), *MessageType->GetName());
				continue;
			}

			NX_VLOG_SUB(GetAgentActor(), LogNexus, Log, TEXT("이벤트 발생: EventTag=%s"), *EventTag.ToString());
			Callback(EventTag, Message, MessageType);
		}
	}
}


FNexusEventCallbackHandle UNexusEventManagerComponent::BindEventCallbackDirect(FGameplayTag EventTag, TFunction<void(FGameplayTag, const void*)>&& Callbacks, UScriptStruct* MessageType)
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

void UNexusEventManagerComponent::FEventCallback::operator()(FGameplayTag EventTag, const void* MessagePtr, UScriptStruct* InMessageType) const
{
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
