// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NexusDataReplication.h"
#include "NexusPrediction.h"
#include "Action/NexusActionDef.h"
#include "Action/NexusAgentInfo.h"
#include "Components/ActorComponent.h"
#include "Cue/NexusCue.h"
#include "Event/NexusEventManagerComponent.h"
#include "Event/NexusEventMessage.h"
#include "SideEffect/NexusSideEffect.h"
#include "SideEffect/NexusSideEffectDef.h"
#include "NexusActionComponent.generated.h"


class UNexusActionComponent;
class UNexusProperty;
class UNexusSideEffect;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnNexusTargetDataSetSignature, FNexusTargetDataHandle /* TargetDataHandle */);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NEXUSACTION_API UNexusActionComponent : public UActorComponent, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	UNexusActionComponent();

	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	

	void InitActionComponent(AActor* InAgentActor);
	void ReleaseActionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	FNexusActionDefHandle AuthAddAction(const FNexusActionDef& ActionDef);
	void AuthRemoveAction(const FNexusActionDefHandle& ActionDefHandle);
	void AuthRemoveAllActions();

	UFUNCTION(BlueprintCallable)
	FNexusActionDefHandle FindActionDefHandle(TSubclassOf<UNexusAction> ActionClass, UObject* SourceObject) const;

	UFUNCTION(BlueprintCallable)
	void TryTriggerAction(FNexusActionDefHandle ActionDefHandle, const FNexusEventMessage& EventMessage);


	UFUNCTION(Server, Reliable)
	void ServerSendNetSyncPoint(FNexusActionDefHandle Handle, FNexusPredictionTag PrimaryPredictionTag, FNexusPredictionTag PredictionTag);
	void CallOrAddNetsyncPointDelegate(FNexusActionDefHandle Handle, FNexusPredictionTag PrimaryPredictionTag, FSimpleMulticastDelegate::FDelegate&& Delegate);

	UFUNCTION(Server, Reliable)
	void ServerSendTargetData(FNexusActionDefHandle Handle, FNexusPredictionTag PrimaryPredictionTag, FNexusPredictionTag PredictionTag, FNexusTargetDataHandle TargetDataHandle);
	void CallOrAddTargetDataDelegate(FNexusActionDefHandle Handle, FNexusPredictionTag PrimaryPredictionTag, FOnNexusTargetDataSetSignature::FDelegate&& Delegate);


	void ReplicatedNetPredictionTag(const FNexusPredictionTag& PredictionTag);


	void IncreaseActionListLock();
	void DecreaseActionListLock();


	UFUNCTION(BlueprintCallable, BlueprintPure)
	static UNexusActionComponent* GetActionComponentFromActor(AActor* Actor);


	UFUNCTION(BlueprintCallable)
	static FNexusSideEffectDef MakeSideEffectDef(UNexusAction* Action, TSubclassOf<UNexusSideEffect> SideEffectClass);
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Trigger Side Effect"))
	static void BP_TriggerSideEffectToActor(UNexusAction* Action, AActor* SideEffectTarget, TSubclassOf<UNexusSideEffect> SideEffectClass);
	void TriggerSideEffect(TSubclassOf<UNexusSideEffect> SideEffectClass, UNexusAction* Action);
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Trigger Side Effect By Def"))
	static void BP_TriggerSideEffectToActorByDef(UNexusAction* Action, AActor* SideEffectTarget, const FNexusSideEffectDef& SideEffectDef);
	void TriggerSideEffectByDef(const FNexusSideEffectDef& SideEffectDef, UNexusAction* Action);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Trigger Cue"))
	static void BP_TriggerCue(UNexusAction* Action, TSubclassOf<UNexusCue> CueClass, FNexusTargetDataHandle TargetDataHandle);
	void TriggerCue(UNexusAction* Action, TSubclassOf<UNexusCue> CueClass, FNexusTargetDataHandle TargetDataHandle);
	UFUNCTION(NetMulticast, Unreliable)
	void NetMulticastTriggerCue(TSubclassOf<UNexusCue> CueClass, FNexusTargetDataHandle TargetDataHandle, FNexusPredictionTag PredictionTag);
	void InternalTriggerCue(TSubclassOf<UNexusCue> CueClass, FNexusTargetDataHandle TargetDataHandle);

	TWeakPtr<FNexusAgentInfo> GetAgentInfo() const { return AgentInfo; }
	FNexusPredictionTagContainer& GetNetPredictionTags() { return NetPredictionTags; }


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

	const FNexusActionDefContainer& GetActionDefs() const { return ActionDefs; }
	FNexusActionDef* FindActionDefByHandle(FNexusActionDefHandle Handle);

private:
	static void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y);
	void InternalOnShowDebugInfo(AActor* DebugTarget, AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y);

	bool HasActionTriggerAuthority(UNexusAction* Action) const;

	void OnActionDefAdded(FNexusActionDef& ActionDef);
	void OnActionDefRemoved(FNexusActionDef& ActionDef);


	void HandleTriggerableActionOnAdded(const FNexusActionDef& NewActionDef);
	void HandleTriggerableActionOnRemoved(const FNexusActionDef& ActionDef);
	void BindActionTriggerEvent(const FNexusActionDef& NewActionDef);
	void UnbindActionTriggerEvent(const FNexusActionDef& ActionDef);
	void OnActionEventTriggered(FGameplayTag GameplayTag, const FNexusEventMessage& EventMessage, FNexusActionDefHandle ActionDefHandle);
	void OnActionEnded(FNexusActionDefHandle ActionDefHandle, UNexusAction* Action);
	


	bool CanTriggerAction(const FNexusActionDef& ActionDef) const;
	void LocalTriggerAction(FNexusActionDef* ActionDef);
	UFUNCTION(Reliable, Server)
	void ServerTryTriggerAction(FNexusActionDefHandle ActionDefHandle, const FNexusEventMessageReplicated& EventMessageReplicated, FNexusPredictionTag PredictionTag);
	UFUNCTION(Reliable, Client)
	void ClientTriggerAction(FNexusActionDefHandle ActionDefHandle, const FNexusEventMessageReplicated& EventMessageReplicated, FNexusPredictionTag PredictionTag);
	UFUNCTION(Reliable, Client)
	void ClientTriggerActionRequestFailed(FNexusActionDefHandle ActionDefHandle, FNexusPredictionTag PredictionTag);

	UFUNCTION(Reliable, Server)
	void ServerRemoteRequestTryTriggerAction(FNexusActionDefHandle ActionDefHandle, const FNexusEventMessage& EventMessage);
	UFUNCTION(Reliable, Client)
	void ClientRemoteRequestTryTriggerAction(FNexusActionDefHandle ActionDefHandle, const FNexusEventMessage& EventMessage);

public:
	FNexusPredictionTag CurrentPredictionTag;

private:
	int32 ActionScopeLockCount = 0;
	TArray<FNexusActionDef> ActionPendingAdds;
	TArray<FNexusActionDefHandle> ActionPendingRemoves;

	TSharedPtr<FNexusAgentInfo> AgentInfo;

	TMap<FNexusActionDefHandle, TArray<FNexusEventCallbackHandle>> BoundedActionEventHandles;

	UPROPERTY(Replicated)
	FNexusActionDefContainer ActionDefs;

	UPROPERTY(Replicated)
	FNexusPredictionTagContainer NetPredictionTags;

	UPROPERTY(Replicated)
	FNexusSideEffectDefContainer SideEffectDefs;

	UPROPERTY(Replicated)
	TArray<TObjectPtr<UNexusProperty>> Properties;


	struct FNexusRepDataDelegate
	{
		FNexusRepDataDelegate(const FNexusPredictionTag& InPredictionTag)
			: PredictionTag(InPredictionTag)
		{
		}


		FNexusPredictionTag PredictionTag;
	};

	struct FNexusNetSyncDelegate : public FNexusRepDataDelegate
	{
		FNexusNetSyncDelegate(const FNexusPredictionTag& InPredictionTag)
			: FNexusRepDataDelegate(InPredictionTag)
		{
		}


		FNexusNetSyncDelegate(const FNexusPredictionTag& InPredictionTag, FSimpleMulticastDelegate::FDelegate&& InDelegate)
			: FNexusRepDataDelegate(InPredictionTag)
		{
			OnSyncDelegate.Add(MoveTemp(InDelegate));
		}

		FSimpleMulticastDelegate OnSyncDelegate;
	};


	TMap<FNexusRepDataKey, FNexusNetSyncDelegate> NetSyncPointDelegates;


	struct FNexusTargetDataDelegate : public FNexusRepDataDelegate
	{
		FNexusTargetDataDelegate(const FNexusPredictionTag& InPredictionTag, const FNexusTargetDataHandle& InTargetDataHandle)
			: FNexusRepDataDelegate(InPredictionTag)
			  , TargetDataHandle(InTargetDataHandle)
		{
		}

		FNexusTargetDataDelegate(FOnNexusTargetDataSetSignature::FDelegate&& InDelegate)
			: FNexusRepDataDelegate(FNexusPredictionTag()),
			  TargetDataHandle()
		{
			OnSetDelegate.Add(MoveTemp(InDelegate));
		}


		FOnNexusTargetDataSetSignature OnSetDelegate;
		FNexusTargetDataHandle TargetDataHandle;
	};

	TMap<FNexusRepDataKey, FNexusTargetDataDelegate> TargetDataDelegates;
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
