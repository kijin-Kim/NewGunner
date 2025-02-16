// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NexusActionDef.h"
#include "NexusAgentInfo.h"
#include "NexusCue.h"
#include "NexusEventMessage.h"
#include "NexusPrediction.h"
#include "NexusSideEffect.h"
#include "NexusSideEffectDef.h"
#include "Components/ActorComponent.h"
#include "Event/NexusEventManagerComponent.h"
#include "NexusLog.h"
#include "NexusActionComponent.generated.h"


class UNexusActionComponent;
class UNexusProperty;
class UNexusSideEffect;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnNexusRepDataSetSignature, FNexusRepDataHandle /* RepDataHandle */);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NEXUSACTION_API UNexusActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNexusActionComponent();

	template <typename T>
	T* NewAction(const UClass* Class, FNexusActionDefHandle ActionDefinitionHandle, TWeakPtr<FNexusAgentInfo> AgentInfo)
	{
		T* NewAction = NewObject<UNexusAction>(GetOwner(), Class);
		NewAction->OnActionEndedDelegate.AddUObject(this, &UNexusActionComponent::OnActionEnded);
		NewAction->InitializeAction(ActionDefinitionHandle, AgentInfo);
		NewAction->OnActionAdded();
		NX_LOG_SUB(LogNexusAction, Verbose, TEXT("Action [%s] 생성 및 추가"), *NewAction->GetName());
		return NewAction;
	}

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	void InitActionComponent(AActor* InAgentActor);
	void ReleaseActionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	FNexusActionDefHandle AuthAddAction(const FNexusActionDef& ActionDefinition);
	void AuthRemoveAction(const FNexusActionDefHandle& ActionDefinitionHandle);
	void AuthRemoveAllActions();

	void TryTriggerAction(FNexusActionDefHandle ActionDefinitionHandle, const FNexusEventMessage& EventMessage);


	UFUNCTION(Server, Reliable)
	void ServerSendNetSyncPoint(FNexusActionDefHandle Handle, FNexusPredictionTag InitPredictionHandle, FNexusPredictionTag NewPredictionHandle);
	void CallOrAddSNetyncPointDelegate(FNexusActionDefHandle Handle, FNexusPredictionTag InitPredictionHandle, FSimpleMulticastDelegate::FDelegate&& Delegate);
	void ReplicatedNetPredictionHandle(const FNexusPredictionTag& PredictionHandle);


	void IncreaseActionListLock();
	void DecreaseActionListLock();

	UFUNCTION(BlueprintCallable)
	static UNexusActionComponent* GetActionComponentFromActor(AActor* Actor);


	UFUNCTION(BlueprintCallable)
	static FNexusSideEffectDef MakeSideEffectDefinition(UNexusAction* Action, TSubclassOf<UNexusSideEffect> SideEffectClass);
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Trigger Side Effect"))
	static void BP_TriggerSideEffectToActor(UNexusAction* Action, AActor* SideEffectTarget, TSubclassOf<UNexusSideEffect> SideEffectClass);
	void TriggerSideEffect(TSubclassOf<UNexusSideEffect> SideEffectClass, UNexusAction* Action);
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Trigger Side Effect By Definition"))
	static void BP_TriggerSideEffectToActorByDefinition(UNexusAction* Action, AActor* SideEffectTarget, const FNexusSideEffectDef& SideEffectDefinition);
	void TriggerSideEffectByDefinition(const FNexusSideEffectDef& SideEffectDefinition, UNexusAction* Action);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Signal"))
	static void BP_Signal(UNexusAction* Action, TSubclassOf<UNexusCue> SignClass, FNexusRepDataHandle TargetData);
	void Signal(UNexusAction* Action, TSubclassOf<UNexusCue> SignClass, FNexusRepDataHandle TargetData);
	UFUNCTION(NetMulticast, Unreliable)
	void NetMulticastSignal(TSubclassOf<UNexusCue> SignClass, FNexusPredictionTag PredictionHandle, FNexusRepDataHandle TargetData);
	void InternalSignal(TSubclassOf<UNexusCue> SignClass, FNexusRepDataHandle TargetData);

	TWeakPtr<FNexusAgentInfo> GetAgentInfo() const { return AgentInfo; }
	FNexusPredictionTagContainer& GetNetPredictionHandles() { return NetPredictionHandles; }


	void AuthAddProperty(FGameplayTag Tag, float Value);
	void AuthRemoveProperty(FGameplayTag Tag);
	void AuthRemoveAllProperties();
	UFUNCTION(BlueprintCallable)
	UNexusProperty* GetProperty(FGameplayTag Tag);

	void AddStaticOperation(FGameplayTag Tag, FNexusPropertyOperation Operation);
	void AddDynamicOperation(FGameplayTag Tag, FNexusPropertyOperation Operation);
	void RemoveOperationByHandle(FGameplayTag Tag, const FNexusPropertyOperationHandle& OperationHandle);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static UNexusProperty* GetPropertyFromActor(AActor* Actor, FGameplayTag Tag);
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static float GetPropertyValueFromActor(AActor* Actor, FGameplayTag Tag);

private:
	static void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y);
	void InternalOnShowDebugInfo(AActor* DebugTarget, AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y);

	bool HasActionTriggerAuthority(UNexusAction* Action) const;

	void OnActionDefinitionAdded(FNexusActionDef& ActionDefinition);
	void OnActionDefinitionRemoved(FNexusActionDef& ActionDefinition);


	void HandleTriggerableActionOnAdded(const FNexusActionDef& NewActionDefinition);
	void HandleTriggerableActionOnRemoved(const FNexusActionDef& ActionDefinition);
	void BindActionTriggerEvent(const FNexusActionDef& NewActionDefinition);
	void UnbindActionTriggerEvent(const FNexusActionDef& ActionDefinition);
	void OnActionEventTriggered(FGameplayTag GameplayTag, const FNexusEventMessage& EventMessage, FNexusActionDefHandle ActionDefinitionHandle);
	void OnActionEnded(FNexusActionDefHandle ActionDefinitionHandle, UNexusAction* Action);
	FNexusActionDef* FindActionDefinitionByHandle(FNexusActionDefHandle ActionDefinitionHandle);

	bool CanTriggerAction(const FNexusActionDef& ActionDefinition) const;
	void LocalTriggerAction(FNexusActionDef* ActionDefinition, FNexusPredictionTag PredictionHandle = FNexusPredictionTag());
	UFUNCTION(Reliable, Server)
	void ServerTryTriggerAction(FNexusActionDefHandle ActionDefinitionHandle, const FNexusEventMessageReplicated& EventMessageReplicated, FNexusPredictionTag PredictionHandle);
	UFUNCTION(Reliable, Client)
	void ClientTriggerAction(FNexusActionDefHandle ActionDefinitionHandle, const FNexusEventMessageReplicated& EventMessageReplicated);
	UFUNCTION(Reliable, Client)
	void ClientTriggerActionRequestSucceeded(FNexusActionDefHandle ActionDefinitionHandle, FNexusPredictionTag PredictionHandle);
	UFUNCTION(Reliable, Client)
	void ClientTriggerActionRequestFailed(FNexusActionDefHandle ActionDefinitionHandle, FNexusPredictionTag PredictionHandle);

	UFUNCTION(Reliable, Server)
	void ServerRemoteRequestTryTriggerAction(FNexusActionDefHandle ActionDefinitionHandle, const FNexusEventMessage& EventMessage);
	UFUNCTION(Reliable, Client)
	void ClientRemoteRequestTryTriggerAction(FNexusActionDefHandle ActionDefinitionHandle, const FNexusEventMessage& EventMessage);

public:
	FNexusPredictionTag CurrentPredictionTag;

private:
	int32 ActionScopeLockCount = 0;
	TArray<FNexusActionDef> ActionPendingAdds;
	TArray<FNexusActionDefHandle> ActionPendingRemoves;

	TSharedPtr<FNexusAgentInfo> AgentInfo;

	FGameplayTagContainer OwnedTags;

	TMap<FNexusActionDefHandle, TArray<FNexusEventCallbackHandle>> BoundedActionEventHandles;

	UPROPERTY(Replicated)
	FNexusActionDefContainer ActionDefinitions;

	UPROPERTY(Replicated)
	FNexusPredictionTagContainer NetPredictionHandles;

	UPROPERTY(Replicated)
	FNexusSideEffectDefContainer SideEffectDefinitions;

	UPROPERTY(Replicated)
	TArray<TObjectPtr<UNexusProperty>> Properties;


	struct FNetSyncPointDelegate
	{
		typedef TPair<FNexusActionDefHandle, FNexusPredictionTag> SyncPointDelegateKeyType;

		FNetSyncPointDelegate(const SyncPointDelegateKeyType& InKey, FSimpleMulticastDelegate::FDelegate&& InDelegate)
			: Key(InKey)
		{
			OnSyncDelegate.Add(InDelegate);
		}

		FNetSyncPointDelegate(const SyncPointDelegateKeyType& InKey)
			: Key(InKey)
		{
		}

		FNetSyncPointDelegate(const SyncPointDelegateKeyType& InKey, FNexusPredictionTag InNewPredictionHandle)
			: Key(InKey),
			  NewPredictionHandle(InNewPredictionHandle)
		{
		}

		bool operator==(const FNetSyncPointDelegate& Other) const
		{
			return Key == Other.Key;
		}

		SyncPointDelegateKeyType Key;
		FNexusPredictionTag NewPredictionHandle;
		FSimpleMulticastDelegate OnSyncDelegate;
	};

	TArray<FNetSyncPointDelegate> NetSyncPointDelegates;
};

struct FNexusActionListScopeLock
{
	FNexusActionListScopeLock(UNexusActionComponent& InActionComponent): ActionComponent(InActionComponent)
	{
		ActionComponent.IncreaseActionListLock();
	}

	~FNexusActionListScopeLock()
	{
		ActionComponent.DecreaseActionListLock();
	}

	UNexusActionComponent& ActionComponent;
};

#define ACTION_LIST_SCOPE_LOCK() FNexusActionListScopeLock ActionListScopeLock(*this)
