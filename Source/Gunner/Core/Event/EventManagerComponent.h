// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "EventManagerComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UEventManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEventManagerComponent();

	// Free function version
	template <typename FMessageStruct>
	void BindEventCallback(FGameplayTag EventTag, TFunction<void(FGameplayTag, const FMessageStruct&)>&& Callback)
	{
		if (EventTag.IsValid())
		{
			EventCallbacks.FindOrAdd(EventTag).Add([InnerFunction = MoveTemp(Callback)](FGameplayTag EventTag, const void* Function)
			{
				InnerFunction(EventTag, *static_cast<const FMessageStruct*>(Function));
			});
		}
	}

	// Member function version
	template <typename FMessageStruct, typename TOwner>
	void BindEventCallback(FGameplayTag EventTag, TOwner* Object, void (TOwner::*Function)(FGameplayTag, const FMessageStruct&))
	{
		if (EventTag.IsValid())
		{
			TWeakObjectPtr<TOwner> Weak = Object;
			EventCallbacks.FindOrAdd(EventTag).Add([Weak, Function](FGameplayTag Tag, const void* Message)
			{
				if (TOwner* Strong = Weak.Get())
				{
					(Strong->*Function)(Tag, *static_cast<const FMessageStruct*>(Message));
				}
			});
		}
	}

	template <typename FMessageStruct>
	void HandleEvent(FGameplayTag EventTag, const FMessageStruct& Message)
	{
		if (EventCallbacks.Contains(EventTag))
		{
			for (const auto& Callback : EventCallbacks[EventTag])
			{
				Callback(EventTag, &Message);
			}
		}
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
	TMap<FGameplayTag, TArray<TFunction<void(FGameplayTag, const void*)>>> EventCallbacks;
};
