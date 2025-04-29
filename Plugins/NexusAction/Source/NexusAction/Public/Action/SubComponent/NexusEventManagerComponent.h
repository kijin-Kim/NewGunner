// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Action/SubComponent/NexusAgentBoundComponent.h"
#include "Components/ActorComponent.h"
#include "NexusEventManagerComponent.generated.h"


class UNexusEventManagerComponent;

USTRUCT()
struct FNexusEventCallbackHandle
{
	GENERATED_BODY()

	FNexusEventCallbackHandle() = default;

public:
	bool IsValid() const { return ID != 0; }

private:
	friend UNexusEventManagerComponent;

	FNexusEventCallbackHandle(int32 InID, FGameplayTag InEventTag) : ID(InID), EventTag(InEventTag)
	{
	}

	int32 ID = 0;
	FGameplayTag EventTag;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NEXUSACTION_API UNexusEventManagerComponent : public UNexusAgentBoundComponent
{
	GENERATED_BODY()

	friend class UNexusAsync_WaitForEvent;

public:
	UNexusEventManagerComponent();
	
	// Free function version
	template <typename FMessageStruct, typename... VarType>
	FNexusEventCallbackHandle BindEventCallback(FGameplayTag EventTag, void (*FreeFunction)(FGameplayTag, const FMessageStruct&, VarType...), VarType... Vars)
	{
		return BindEventCallbackDirect(EventTag, [FreeFunction, Vars...](FGameplayTag EventTag, const void* MessagePtr)
		{
			FreeFunction(EventTag, *static_cast<const FMessageStruct*>(MessagePtr), Vars...);
		}, TBaseStructure<FMessageStruct>::Get());
	}

	// Member function version
	template <typename FMessageStruct, typename TOwner, typename... VarType>
	FNexusEventCallbackHandle BindEventCallback(FGameplayTag EventTag, TOwner* Object, void (TOwner::*Function)(FGameplayTag, const FMessageStruct&, VarType...), VarType... Vars)
	{
		TWeakObjectPtr<TOwner> Weak = Object;
		return BindEventCallbackDirect(EventTag, [Weak, Function, Vars...](FGameplayTag Tag, const void* MessagePtr)
		{
			if (TOwner* Strong = Weak.Get())
			{
				(Strong->*Function)(Tag, *static_cast<const FMessageStruct*>(MessagePtr), Vars...);
			}
		}, TBaseStructure<FMessageStruct>::Get());
	}
	
	FNexusEventCallbackHandle BindEventCallbackDirect(FGameplayTag EventTag, TFunction<void(FGameplayTag, const void*)>&& Callbacks, UScriptStruct* MessageType);
	
	void UnbindEventCallback(FNexusEventCallbackHandle Handle);
	void UnbindAllEventCallbacks();
	void HandleEvent(FGameplayTag EventTag, const void* Message, UScriptStruct* MessageType);

private:
	

private:
	struct FEventCallback
	{
		int32 HandleID = 0;
		TFunction<void(FGameplayTag, const void*)> Callback;
		TWeakObjectPtr<UScriptStruct> MessageType;
		
		void operator()(FGameplayTag EventTag, const void* MessagePtr, UScriptStruct* InMessageType) const;
	};

	struct FEventCallbackList
	{
	public:
		int32 HandleID = 0;
		TArray<FEventCallback> Callbacks;

		void IncrementCallbackListLock();
		void DecrementCallbackListLock();
		
		int32 CallbackListScopeLockCount = 0;
		TArray<FEventCallback> CallbackPendingAdds;
		TArray<FNexusEventCallbackHandle> CallbackPendingRemoves;
	};

	struct FNexusEventCallbackListScopeLock
	{
		FNexusEventCallbackListScopeLock(FEventCallbackList& InCallbackList) : CallbackList(InCallbackList)
		{
			CallbackList.IncrementCallbackListLock();
		}
		~FNexusEventCallbackListScopeLock()
		{
			CallbackList.DecrementCallbackListLock();
		}

		FEventCallbackList& CallbackList;
	};

	TMap<FGameplayTag, FEventCallbackList> EventCallbacks;
};
