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

public:
	UEventManagerComponent();

	// Free function version
	template <typename FMessageStruct>
	FEventCallbackHandle BindEventCallback(FGameplayTag EventTag, TFunction<void(FGameplayTag, const FMessageStruct&)>&& Callback)
	{
		return BindEventCallbackInternal(EventTag, [InnerFunction = MoveTemp(Callback)](FGameplayTag EventTag, const void* Function)
		{
			InnerFunction(EventTag, *static_cast<const FMessageStruct*>(Function));
		});
	}

	// Member function version
	template <typename FMessageStruct, typename TOwner>
	FEventCallbackHandle BindEventCallback(FGameplayTag EventTag, TOwner* Object, void (TOwner::*Function)(FGameplayTag, const FMessageStruct&))
	{
		TWeakObjectPtr<TOwner> Weak = Object;
		return BindEventCallbackInternal(EventTag, [Weak, Function](FGameplayTag Tag, const void* Message)
		{
			if (TOwner* Strong = Weak.Get())
			{
				(Strong->*Function)(Tag, *static_cast<const FMessageStruct*>(Message));
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

		void operator()(FGameplayTag EventTag, const void* Message) const
		{
			Callback(EventTag, Message);
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
