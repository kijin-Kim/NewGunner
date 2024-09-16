// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "EventManagerComponent.generated.h"


class UEventManagerComponent;

USTRUCT()
struct FEventCallbackHandle
{
	GENERATED_BODY()

	FEventCallbackHandle() = default;

public:
	bool IsValid() const { return ID != 0; }

private:
	friend UEventManagerComponent;

	FEventCallbackHandle(int32 InID, FGameplayTag InEventTag) : ID(InID), EventTag(InEventTag)
	{
	}

	int32 ID = 0;
	FGameplayTag EventTag;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UEventManagerComponent : public UActorComponent
{
	GENERATED_BODY()

	friend class UAsyncAction_WaitForGunnerEvent;

public:
	UEventManagerComponent();
	
	// Free function version
	template <typename FMessageStruct, typename... VarType>
	FEventCallbackHandle BindEventCallback(FGameplayTag EventTag, void (*FreeFunction)(FGameplayTag, const FMessageStruct&, VarType...), VarType... Vars)
	{
		return BindEventCallbackInternal(EventTag, [FreeFunction, Vars...](FGameplayTag EventTag, const void* MessagePtr)
		{
			FreeFunction(EventTag, *static_cast<const FMessageStruct*>(MessagePtr), Vars...);
		});
	}

	// Member function version
	template <typename FMessageStruct, typename TOwner, typename... VarType>
	FEventCallbackHandle BindEventCallback(FGameplayTag EventTag, TOwner* Object, void (TOwner::*Function)(FGameplayTag, const FMessageStruct&, VarType...), VarType... Vars)
	{
		TWeakObjectPtr<TOwner> Weak = Object;
		return BindEventCallbackInternal(EventTag, [Weak, Function, Vars...](FGameplayTag Tag, const void* MessagePtr)
		{
			if (TOwner* Strong = Weak.Get())
			{
				(Strong->*Function)(Tag, *static_cast<const FMessageStruct*>(MessagePtr), Vars...);
			}
		});
	}


	void UnbindEventCallback(FEventCallbackHandle Handle);


	template <typename FMessageStruct>
	void HandleEvent(FGameplayTag EventTag, const FMessageStruct& Message)
	{
		if (EventCallbacks.Contains(EventTag))
		{
			bIsIterating = true;
			for (const auto& Callback : EventCallbacks[EventTag].Callbacks)
			{
				Callback(EventTag, &Message);
			}
			bIsIterating = false;
		}

		AddPendingEventCallbacks();
		RemovePendingEventCallbacks();
	}

	template <typename FMessageStruct>
	static void SendEventToActor(FGameplayTag EventTag, const FMessageStruct& Message, AActor* TargetActor)
	{
		if (UEventManagerComponent* EventManagerComponent = TargetActor->GetComponentByClass<UEventManagerComponent>())
		{
			EventManagerComponent->HandleEvent<FMessageStruct>(EventTag, Message);
		}
	}

private:
	FEventCallbackHandle BindEventCallbackInternal(FGameplayTag EventTag, TFunction<void(FGameplayTag, const void*)>&& Callbacks);
	void AddPendingEventCallbacks();
	void RemovePendingEventCallbacks();

private:
	struct FEventCallback
	{
		int32 HandleID = 0;
		TFunction<void(FGameplayTag, const void*)> Callback;
		bool bIsPendingRemove = false;

		void operator()(FGameplayTag EventTag, const void* MessagePtr) const
		{
			Callback(EventTag, MessagePtr);
		}
	};

	struct FEventCallbackList
	{
		int32 HandleID = 0;
		TArray<FEventCallback> Callbacks;
	};

	TMap<FGameplayTag, FEventCallbackList> EventCallbacks;
	TMap<FGameplayTag, TArray<FEventCallback>> PendingAddCallbacks;
	bool bIsIterating = false;
};
