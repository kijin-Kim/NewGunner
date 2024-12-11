// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerEventManagerComponent.h"

#include "GunnerEventManagerInterface.h"
#include "Gunner/Gunner.h"


UGunnerEventManagerComponent::UGunnerEventManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGunnerEventManagerComponent::UnbindEventCallback(FGunnerEventCallbackHandle Handle)
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

void UGunnerEventManagerComponent::UnbindAllEventCallbacks()
{
	for (auto& EventCallback : EventCallbacks)
	{
		FGunnerEventCallbackListScopeLock CallbackListScopeLock(EventCallback.Value);
		for (const auto& Callback : EventCallback.Value.Callbacks)
		{
			UnbindEventCallback({Callback.HandleID, EventCallback.Key});
		}
	}
}

void UGunnerEventManagerComponent::HandleEvent(FGameplayTag EventTag, const void* Message, UScriptStruct* MessageType)
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

UGunnerEventManagerComponent* UGunnerEventManagerComponent::GetEventManagerComponentFromActor(AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}

	if (IGunnerEventManagerInterface* GunnerEventManagerComponentInterface = Cast<IGunnerEventManagerInterface>(Actor))
	{
		return GunnerEventManagerComponentInterface->GetEventManagerComponent();
	}

	if (UGunnerEventManagerComponent* EventManagerComponent = Actor->GetComponentByClass<UGunnerEventManagerComponent>())
	{
		return EventManagerComponent;
	}

	return nullptr;
}

FGunnerEventCallbackHandle UGunnerEventManagerComponent::BindEventCallbackInternal(FGameplayTag EventTag, TFunction<void(FGameplayTag, const void*)>&& Callbacks, UScriptStruct* MessageType)
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
		return FGunnerEventCallbackHandle(EventCallbacks[EventTag].HandleID, EventTag);
	}

	EventCallbackList.Callbacks.Add({EventCallbackList.HandleID, MoveTemp(Callbacks), MessageType});
	return FGunnerEventCallbackHandle(EventCallbacks[EventTag].HandleID, EventTag);
}


DEFINE_FUNCTION(UGunnerEventManagerComponent::execBP_SendEventToActor)
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

	if (UGunnerEventManagerComponent* EventManagerComponent = GetEventManagerComponentFromActor(TargetActor))
	{
		EventManagerComponent->HandleEvent(EventTag, MessagePtr, StructProperty->Struct);
	}
}


void UGunnerEventManagerComponent::FEventCallback::operator()(FGameplayTag EventTag, const void* MessagePtr, UScriptStruct* InMessageType) const
{
	if (MessageType != InMessageType)
	{
		UE_LOG(LogGunner, Error, TEXT("Message type mismatch"));
		return;
	}
	Callback(EventTag, MessagePtr);
}

void UGunnerEventManagerComponent::FEventCallbackList::IncrementCallbackListLock()
{
	CallbackListScopeLockCount++;
}

void UGunnerEventManagerComponent::FEventCallbackList::DecrementCallbackListLock()
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
