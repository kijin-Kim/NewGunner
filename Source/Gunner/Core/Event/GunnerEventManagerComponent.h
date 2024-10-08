// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "GunnerEventManagerComponent.generated.h"


class UGunnerEventManagerComponent;

USTRUCT()
struct FGunnerEventCallbackHandle
{
	GENERATED_BODY()

	FGunnerEventCallbackHandle() = default;

public:
	bool IsValid() const { return ID != 0; }

private:
	friend UGunnerEventManagerComponent;

	FGunnerEventCallbackHandle(int32 InID, FGameplayTag InEventTag) : ID(InID), EventTag(InEventTag)
	{
	}

	int32 ID = 0;
	FGameplayTag EventTag;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UGunnerEventManagerComponent : public UActorComponent
{
	GENERATED_BODY()

	friend class UGunnerActionAsync_WaitForGunnerEvent;

public:
	UGunnerEventManagerComponent();
	
	// Free function version
	template <typename FMessageStruct, typename... VarType>
	FGunnerEventCallbackHandle BindEventCallback(FGameplayTag EventTag, void (*FreeFunction)(FGameplayTag, const FMessageStruct&, VarType...), VarType... Vars)
	{
		return BindEventCallbackInternal(EventTag, [FreeFunction, Vars...](FGameplayTag EventTag, const void* MessagePtr)
		{
			FreeFunction(EventTag, *static_cast<const FMessageStruct*>(MessagePtr), Vars...);
		}, TBaseStructure<FMessageStruct>::Get());
	}

	// Member function version
	template <typename FMessageStruct, typename TOwner, typename... VarType>
	FGunnerEventCallbackHandle BindEventCallback(FGameplayTag EventTag, TOwner* Object, void (TOwner::*Function)(FGameplayTag, const FMessageStruct&, VarType...), VarType... Vars)
	{
		TWeakObjectPtr<TOwner> Weak = Object;
		return BindEventCallbackInternal(EventTag, [Weak, Function, Vars...](FGameplayTag Tag, const void* MessagePtr)
		{
			if (TOwner* Strong = Weak.Get())
			{
				(Strong->*Function)(Tag, *static_cast<const FMessageStruct*>(MessagePtr), Vars...);
			}
		}, TBaseStructure<FMessageStruct>::Get());
	}


	void UnbindEventCallback(FGunnerEventCallbackHandle Handle);
	void HandleEvent(FGameplayTag EventTag, const void* Message, UScriptStruct* MessageType);

	UFUNCTION(BlueprintCallable)
	static UGunnerEventManagerComponent* GetEventManagerComponentFromActor(AActor* Actor);

	template <typename FMessageStruct>
	static void SendEventToActor(FGameplayTag EventTag, const FMessageStruct& Message, AActor* TargetActor)
	{
		if (UGunnerEventManagerComponent* EventManagerComponent = GetEventManagerComponentFromActor(TargetActor))
		{
			EventManagerComponent->HandleEvent(EventTag, &Message, TBaseStructure<FMessageStruct>::Get());
		}
	}

	UFUNCTION(BlueprintCallable, CustomThunk, meta = (CustomStructureParam = "Message", DisplayName= "Send Event To Actor"))
	static void BP_SendEventToActor(FGameplayTag EventTag, AActor* TargetActor, const int32& Message);
	DECLARE_FUNCTION(execBP_SendEventToActor);


private:
	FGunnerEventCallbackHandle BindEventCallbackInternal(FGameplayTag EventTag, TFunction<void(FGameplayTag, const void*)>&& Callbacks, UScriptStruct* MessageType);

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
		TArray<FGunnerEventCallbackHandle> CallbackPendingRemoves;
	};

	struct FGunnerEventCallbackListScopeLock
	{
		FGunnerEventCallbackListScopeLock(FEventCallbackList& InCallbackList) : CallbackList(InCallbackList)
		{
			CallbackList.IncrementCallbackListLock();
		}
		~FGunnerEventCallbackListScopeLock()
		{
			CallbackList.DecrementCallbackListLock();
		}

		FEventCallbackList& CallbackList;
	};

	TMap<FGameplayTag, FEventCallbackList> EventCallbacks;
};
