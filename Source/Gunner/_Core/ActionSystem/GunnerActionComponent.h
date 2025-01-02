// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GunnerActionAgentInfo.h"
#include "GunnerActionDefinition.h"
#include "GunnerActionSideEffect.h"
#include "GunnerActionSideEffectDefinition.h"
#include "GunnerActionSign.h"
#include "AsyncAction/GunnerActionNetPrediction.h"
#include "Components/ActorComponent.h"
#include "Gunner/_Core/Event/GunnerEventManagerComponent.h"
#include "Gunner/_Core/Input/GunnerEventMessage.h"
#include "GunnerActionComponent.generated.h"


class UGunnerActionComponent;
class UGunnerActionProperty;
class UGunnerActionSideEffect;

USTRUCT()
struct FGunnerActionPropertyMapping
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FGameplayTag Tag;
	UPROPERTY(EditAnywhere)
	float Value = 0.0f;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UGunnerActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGunnerActionComponent();

	template <typename T>
	T* NewGunnerAction(UObject* Outer, const UClass* Class, FGunnerActionDefinitionHandle ActionDefinitionHandle, TWeakPtr<FGunnerActionAgentInfo> AgentInfo)
	{
		T* NewAction = NewObject<UGunnerAction>(GetOwner(), Class);
		NewAction->OnGunnerActionEndedDelegate.AddUObject(this, &UGunnerActionComponent::OnActionEnded);
		NewAction->InitializeGunnerAction(ActionDefinitionHandle, AgentInfo);
		return NewAction;
	}

	void InitActionComponent(AActor* InOwnerActor, AActor* InAgentActor);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void ReadyForReplication() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	FGunnerActionDefinitionHandle AuthAddAction(const FGunnerActionDefinition& ActionDefinition);
	void AuthRemoveAction(const FGunnerActionDefinitionHandle& ActionDefinitionHandle);
	void AuthRemoveAllActions();
	void TryTriggerAction(FGunnerActionDefinitionHandle ActionDefinitionHandle, const FGunnerEventMessage& EventMessage);


	UFUNCTION(Server, Reliable)
	void ServerSendNetSyncPoint(FGunnerActionDefinitionHandle Handle, FGunnerActionNetPredictionHandle InitPredictionHandle, FGunnerActionNetPredictionHandle NewPredictionHandle);
	void CallOrAddSNetyncPointDelegate(FGunnerActionDefinitionHandle Handle, FGunnerActionNetPredictionHandle InitPredictionHandle, FSimpleMulticastDelegate::FDelegate&& Delegate);


	void IncrementActionListLock();
	void DecrementActionListLock();

	UFUNCTION(BlueprintCallable)
	static UGunnerActionComponent* GetActionComponentFromActor(AActor* Actor);

	bool HasActionTriggerAuthority(UGunnerAction* Action) const;

	UFUNCTION(BlueprintCallable)
	static FGunnerActionSideEffectDefinition MakeSideEffectDefinition(TSubclassOf<UGunnerActionSideEffect> SideEffectClass);
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Trigger Side Effect"))
	static void BP_TriggerSideEffectToActor(UGunnerAction* Action, AActor* SideEffectTarget, TSubclassOf<UGunnerActionSideEffect> SideEffectClass);
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Trigger Side Effect With Definition"))
	static void BP_TriggerSideEffectToActorWithSideEffectDefinition(UGunnerAction* Action, AActor* SideEffectTarget, const FGunnerActionSideEffectDefinition& SideEffectDefinition);
	void TriggerSideEffect(const FGunnerActionSideEffectDefinition& NewSideEffectDefinition, UGunnerAction* Action);
	
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Signal"))
	static void BP_Signal(UGunnerAction* Action, TSubclassOf<UGunnerActionSign> SignClass, UObject* SignalDataObject);
	void Signal(UGunnerAction* Action, TSubclassOf<UGunnerActionSign> SignClass, UObject* SignalDataObject);
	UFUNCTION(NetMulticast, Unreliable)
	void NetMulticastSignal(TSubclassOf<UGunnerActionSign> SignClass, UObject* SignalDataObject, FGunnerActionNetPredictionHandle PredictionHandle);
	void InternalSignal(TSubclassOf<UGunnerActionSign> SignClass, UObject* SignalDataObject);

	TWeakPtr<FGunnerActionAgentInfo> GetAgentInfo() const { return AgentInfo; }
	FGunnerActionNetPredictionHandleArray& GetNetPredictionHandleArray() { return NetPredictionHandleArray; }


private:
	static void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y);
	void InternalOnShowDebugInfo(AActor* DebugTarget, AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y);

	void HandleTriggerableActionOnAdded(const FGunnerActionDefinition& NewActionDefinition);
	void HandleTriggerableActionOnRemoved(const FGunnerActionDefinition& ActionDefinition);
	void BindActionTriggerEvent(const FGunnerActionDefinition& NewActionDefinition);
	void UnbindActionTriggerEvent(const FGunnerActionDefinition& ActionDefinition);
	void OnActionEventTriggered(FGameplayTag GameplayTag, const FGunnerEventMessage& EventMessage, FGunnerActionDefinitionHandle ActionDefinitionHandle);
	UFUNCTION()
	void OnRep_ActionDefinitions(const TArray<FGunnerActionDefinition>& OldActionDefinitions);
	void OnActionEnded(FGunnerActionDefinitionHandle ActionDefinitionHandle, UGunnerAction* Action);
	FGunnerActionDefinition* FindActionDefinitionByHandle(FGunnerActionDefinitionHandle ActionDefinitionHandle);

	bool CanTriggerAction(const FGunnerActionDefinition& ActionDefinition, const FGunnerEventMessage& EventMessage) const;
	void LocalTriggerAction(FGunnerActionDefinition* ActionDefinition, FGunnerActionNetPredictionHandle PredictionHandle = FGunnerActionNetPredictionHandle());
	UFUNCTION(Reliable, Server)
	void ServerTryTriggerAction(FGunnerActionDefinitionHandle ActionDefinitionHandle, const FGunnerEventMessageReplicated& EventMessageReplicated, FGunnerActionNetPredictionHandle PredictionHandle);
	UFUNCTION(Reliable, Client)
	void ClientTriggerAction(FGunnerActionDefinitionHandle ActionDefinitionHandle, const FGunnerEventMessageReplicated& EventMessageReplicated);
	UFUNCTION(Reliable, Client)
	void ClientTriggerActionRequestSucceeded(FGunnerActionDefinitionHandle ActionDefinitionHandle, FGunnerActionNetPredictionHandle PredictionHandle);
	UFUNCTION(Reliable, Client)
	void ClientTriggerActionRequestFailed(FGunnerActionDefinitionHandle ActionDefinitionHandle, FGunnerActionNetPredictionHandle PredictionHandle);

	UFUNCTION(Reliable, Server)
	void ServerRemoteRequestTryTriggerAction(FGunnerActionDefinitionHandle ActionDefinitionHandle, const FGunnerEventMessage& EventMessage);
	UFUNCTION(Reliable, Client)
	void ClientRemoteRequestTryTriggerAction(FGunnerActionDefinitionHandle ActionDefinitionHandle, const FGunnerEventMessage& EventMessage);



private:
	UPROPERTY(ReplicatedUsing=OnRep_ActionDefinitions)
	TArray<FGunnerActionDefinition> ActionDefinitions;
	int32 ActionScopeLockCount = 0;
	TArray<FGunnerActionDefinition> ActionPendingAdds;
	TArray<FGunnerActionDefinitionHandle> ActionPendingRemoves;

	TSharedPtr<FGunnerActionAgentInfo> AgentInfo;

	FGameplayTagContainer OwnedTags;

	TMap<FGunnerActionDefinitionHandle, TArray<FGunnerEventCallbackHandle>> BoundedActionEventHandles;
	

	UPROPERTY(Replicated)
	FGunnerActionNetPredictionHandleArray NetPredictionHandleArray;

	UPROPERTY(Replicated)
	FGunnerActionSideEffectDefinitionArray SideEffectDefinitionArray;


public:
	UFUNCTION(BlueprintCallable)
	void AuthAddProperty(FGameplayTag Tag, float Value);
	UFUNCTION(BlueprintCallable)
	void AuthRemoveProperty(FGameplayTag Tag);
	void AuthRemoveAllProperties();
	FGunnerActionProperty* GetProperty(FGameplayTag Tag);
	const TArray<FGunnerActionProperty>& GetProperties() const { return PropertyArray.Items; }
	void OnAdded(const FGunnerActionSideEffectDefinition& SideEffectDefinition, FGunnerActionNetPredictionHandle PredictionHandle);
	void OnRemoved(FGunnerActionSideEffectDefinitionHandle SideEffectDefinitionHandle);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static float GetPropertyValueFromActor(AActor* Actor, FGameplayTag Tag);

	UPROPERTY(EditAnywhere)
	TArray<FGunnerActionPropertyMapping> StartProperties;
	
	UPROPERTY(Replicated)
	FGunnerActionPropertyArray PropertyArray;

	FGunnerActionNetPredictionHandle NetPredictionHandle;

	struct FNetSyncPointDelegate
	{
		typedef TPair<FGunnerActionDefinitionHandle, FGunnerActionNetPredictionHandle> SyncPointDelegateKeyType;

		FNetSyncPointDelegate(const SyncPointDelegateKeyType& InKey, FSimpleMulticastDelegate::FDelegate&& InDelegate)
			: Key(InKey)
		{
			OnSyncDelegate.Add(InDelegate);
		}

		FNetSyncPointDelegate(const SyncPointDelegateKeyType& InKey)
			: Key(InKey)
		{
		}

		FNetSyncPointDelegate(const SyncPointDelegateKeyType& InKey, FGunnerActionNetPredictionHandle InNewPredictionHandle)
			: Key(InKey),
			  NewPredictionHandle(InNewPredictionHandle)
		{
		}

		bool operator==(const FNetSyncPointDelegate& Other) const
		{
			return Key == Other.Key;
		}

		SyncPointDelegateKeyType Key;
		FGunnerActionNetPredictionHandle NewPredictionHandle;
		FSimpleMulticastDelegate OnSyncDelegate;
	};

	TArray<FNetSyncPointDelegate> NetSyncPointDelegates;
	
};

struct FGunnerActionListScopeLock
{
	FGunnerActionListScopeLock(UGunnerActionComponent& InActionComponent): ActionComponent(InActionComponent)
	{
		ActionComponent.IncrementActionListLock();
	}

	~FGunnerActionListScopeLock()
	{
		ActionComponent.DecrementActionListLock();
	}

	UGunnerActionComponent& ActionComponent;
};

#define ACTION_LIST_SCOPE_LOCK() FGunnerActionListScopeLock ActionListScopeLock(*this)
