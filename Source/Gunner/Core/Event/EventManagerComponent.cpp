// Fill out your copyright notice in the Description page of Project Settings.


#include "EventManagerComponent.h"


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

	if (bIsIterating)
	{
		for (auto& Callbacks : EventCallbacks[Handle.EventTag].Callbacks)
		{
			if (Callbacks.HandleID == Handle.ID)
			{
				Callbacks.bIsPendingRemove = true;
			}
		}
	}
	else
	{
		EventCallbacks[Handle.EventTag].Callbacks.RemoveAll([Handle](const FEventCallback& Callback)
		{
			return Callback.HandleID == Handle.ID;
		});
	}
}

FEventCallbackHandle UEventManagerComponent::BindEventCallbackInternal(FGameplayTag EventTag, TFunction<void(FGameplayTag, const void*)>&& Callbacks)
{
	if (!EventTag.IsValid())
	{
		return {};
	}

	auto& EventCallbackList = EventCallbacks.FindOrAdd(EventTag);
	EventCallbackList.HandleID++;

	auto& TargetEventCallbackList = bIsIterating ? PendingAddCallbacks.FindOrAdd(EventTag) : EventCallbackList.Callbacks;
	TargetEventCallbackList.Add({EventCallbackList.HandleID, MoveTemp(Callbacks)});
	return FEventCallbackHandle(EventCallbacks[EventTag].HandleID, EventTag);
}

void UEventManagerComponent::AddPendingEventCallbacks()
{
	for (const auto& [EventTag, Callbacks] : PendingAddCallbacks)
	{
		EventCallbacks[EventTag].Callbacks.Append(Callbacks);
	}
	PendingAddCallbacks.Empty();
}

void UEventManagerComponent::RemovePendingEventCallbacks()
{
	for (auto& [EventTag, Callbacks] : EventCallbacks)
	{
		Callbacks.Callbacks.RemoveAll([](const FEventCallback& Callback)
		{
			return Callback.bIsPendingRemove;
		});
	}
}
