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
		NewAction->OnActionAdded();
		return NewAction;
	}

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	
	void InitActionComponent(AActor* InAgentActor);
	void ReleaseActionComponent();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	FGunnerActionDefinitionHandle AuthAddAction(const FGunnerActionDefinition& ActionDefinition);
	void AuthRemoveAction(const FGunnerActionDefinitionHandle& ActionDefinitionHandle);
	void AuthRemoveAllActions();
	
	void TryTriggerAction(FGunnerActionDefinitionHandle ActionDefinitionHandle, const FGunnerEventMessage& EventMessage);


	UFUNCTION(Server, Reliable)
	void ServerSendNetSyncPoint(FGunnerActionDefinitionHandle Handle, FGunnerActionNetPredictionHandle InitPredictionHandle, FGunnerActionNetPredictionHandle NewPredictionHandle);
	void CallOrAddSNetyncPointDelegate(FGunnerActionDefinitionHandle Handle, FGunnerActionNetPredictionHandle InitPredictionHandle, FSimpleMulticastDelegate::FDelegate&& Delegate);


	void IncreaseActionListLock();
	void DecreaseActionListLock();

	UFUNCTION(BlueprintCallable)
	static UGunnerActionComponent* GetActionComponentFromActor(AActor* Actor);

	
	UFUNCTION(BlueprintCallable)
	static FGunnerActionSideEffectDefinition MakeSideEffectDefinition(UGunnerAction* Action, TSubclassOf<UGunnerActionSideEffect> SideEffectClass);
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Trigger Side Effect"))
	static void BP_TriggerSideEffectToActor(UGunnerAction* Action, AActor* SideEffectTarget, TSubclassOf<UGunnerActionSideEffect> SideEffectClass);
	void TriggerSideEffect(TSubclassOf<UGunnerActionSideEffect> SideEffectClass, UGunnerAction* Action);
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Trigger Side Effect By Definition"))
	static void BP_TriggerSideEffectToActorByDefinition(UGunnerAction* Action, AActor* SideEffectTarget, const FGunnerActionSideEffectDefinition& SideEffectDefinition);
	void TriggerSideEffectByDefinition(const FGunnerActionSideEffectDefinition& SideEffectDefinition, UGunnerAction* Action);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Signal"))
	static void BP_Signal(UGunnerAction* Action, TSubclassOf<UGunnerActionSign> SignClass, UObject* SignalDataObject);
	void Signal(UGunnerAction* Action, TSubclassOf<UGunnerActionSign> SignClass, UObject* SignalDataObject);
	UFUNCTION(NetMulticast, Unreliable)
	void NetMulticastSignal(TSubclassOf<UGunnerActionSign> SignClass, UObject* SignalDataObject, FGunnerActionNetPredictionHandle PredictionHandle);
	void InternalSignal(TSubclassOf<UGunnerActionSign> SignClass, UObject* SignalDataObject);

	TWeakPtr<FGunnerActionAgentInfo> GetAgentInfo() const { return AgentInfo; }
	FGunnerActionNetPredictionHandleArray& GetNetPredictionHandles() { return NetPredictionHandles; }

private:
	static void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y);
	void InternalOnShowDebugInfo(AActor* DebugTarget, AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y);
	
	bool HasActionTriggerAuthority(UGunnerAction* Action) const;

	void OnActionDefinitionAdded(FGunnerActionDefinition& ActionDefinition);
	void OnActionDefinitionRemoved(FGunnerActionDefinition& ActionDefinition);


	void HandleTriggerableActionOnAdded(const FGunnerActionDefinition& NewActionDefinition);
	void HandleTriggerableActionOnRemoved(const FGunnerActionDefinition& ActionDefinition);
	void BindActionTriggerEvent(const FGunnerActionDefinition& NewActionDefinition);
	void UnbindActionTriggerEvent(const FGunnerActionDefinition& ActionDefinition);
	void OnActionEventTriggered(FGameplayTag GameplayTag, const FGunnerEventMessage& EventMessage, FGunnerActionDefinitionHandle ActionDefinitionHandle);
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
	int32 ActionScopeLockCount = 0;
	TArray<FGunnerActionDefinition> ActionPendingAdds;
	TArray<FGunnerActionDefinitionHandle> ActionPendingRemoves;
	
	TSharedPtr<FGunnerActionAgentInfo> AgentInfo;

	FGameplayTagContainer OwnedTags;

	TMap<FGunnerActionDefinitionHandle, TArray<FGunnerEventCallbackHandle>> BoundedActionEventHandles;

	UPROPERTY(Replicated)
	FGunnerActionDefinitionArray ActionDefinitions;

	UPROPERTY(Replicated)
	FGunnerActionNetPredictionHandleArray NetPredictionHandles;

	UPROPERTY(Replicated)
	FGunnerActionSideEffectDefinitionArray SideEffectDefinitions;

public:
	void AuthAddProperty(FGameplayTag Tag, float Value);
	void AuthRemoveProperty(FGameplayTag Tag);
	void AuthRemoveAllProperties();
	UGunnerActionProperty* GetProperty(FGameplayTag Tag);

	void AddStaticOperation(FGameplayTag Tag, FGunnerActionPropertyOperation Operation);
	void AddDynamicOperation(FGameplayTag Tag, FGunnerActionPropertyOperation Operation);
	void RemoveOperationByHandle(FGameplayTag Tag, const FGunnerActionPropertyOperationHandle& OperationHandle);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static float GetPropertyValueFromActor(AActor* Actor, FGameplayTag Tag);
	

	UPROPERTY(Replicated)
	TArray<TObjectPtr<UGunnerActionProperty>> Properties;

	FGunnerActionNetPredictionHandle CurrentNetPredictionHandle;

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
		ActionComponent.IncreaseActionListLock();
	}

	~FGunnerActionListScopeLock()
	{
		ActionComponent.DecreaseActionListLock();
	}

	UGunnerActionComponent& ActionComponent;
};

#define ACTION_LIST_SCOPE_LOCK() FGunnerActionListScopeLock ActionListScopeLock(*this)
