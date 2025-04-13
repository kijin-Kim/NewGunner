// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Action/NexusActionDef.h"
#include "Action/NexusAgentInfo.h"
#include "Action/SubComponent/NexusPredictionComponent.h"
#include "Components/ActorComponent.h"
#include "Cue/NexusCue.h"
#include "Event/NexusEventManagerComponent.h"
#include "Event/NexusEventMessage.h"
#include "Prediction/NexusPrediction.h"
#include "SideEffect/NexusSideEffect.h"
#include "SideEffect/NexusSideEffectDef.h"
#include "NexusActionComponent.generated.h"


class UNexusGameplayTagComponent;
class UNexusPropertyComponent;
class UNexusSideEffectComponent;
struct FNexusTriggerCueParams;
class UNexusCueComponent;
class UNexusActionComponent;
class UNexusProperty;
class UNexusSideEffect;


DECLARE_MULTICAST_DELEGATE(FOnNexusActionComponentSetupCompletedSignature);
DECLARE_MULTICAST_DELEGATE(FOnNexusActionComponentTeardownCompletedSignature);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NEXUSACTION_API UNexusActionComponent : public UActorComponent, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	UNexusActionComponent();
	virtual void InitializeComponent() override;

	static void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y);


	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


	void SetupActionComponent(AActor* InAgentActor);
	void TeardownActionComponent();

	virtual void OnSetupActionComponent()
	{
	}

	void SimTriggerCue(const FNexusTriggerCueParams& CueParams, FNexusLoopingCueHandle CueHandle);

	UFUNCTION(BlueprintCallable)
	FNexusActionDefHandle AuthAddAction(const FNexusActionDef& ActionDef);
	void AuthRemoveAction(const FNexusActionDefHandle& ActionDefHandle);
	void AuthRemoveAllActions();

	void AuthAddProperty(FGameplayTag Tag, float Value);

	UFUNCTION(BlueprintCallable)
	FNexusActionDefHandle FindActionDefHandle(TSubclassOf<UNexusAction> ActionClass, UObject* SourceObject) const;


	UFUNCTION(BlueprintCallable)
	void TryTriggerAction(FNexusActionDefHandle ActionDefHandle, const FNexusEventMessage& EventMessage);

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
	void TriggerSideEffectByDef(const FNexusSideEffectDef& NewSideEffectDef, UNexusAction* Action, FNexusPredictionEventSignature::FDelegate&& OnPredictionEnded = {}, FNexusPredictionEventSignature::FDelegate&& OnPredictionFailed = {}) const;


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Trigger Cue"))
	static void BP_TriggerCue(UNexusAction* Action, TSubclassOf<ANexusCue> CueClass, const FNexusTargetDataHandle& TargetDataHandle);
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, meta = (DisplayName = "Auth End Cue"))
	static void BP_AuthEndCue(UNexusAction* Action, FNexusLoopingCueHandle CueHandle);

	TWeakPtr<FNexusAgentInfo> GetAgentInfo() const { return AgentInfo; }
	bool IsAgentLocallyControlled() const;
	bool IsAgentLocallyPlayerControlled() const;
	bool IsOwnerActorAuthoritative() const;

	AActor* GetAgentActor() const { return AgentInfo.IsValid() ? AgentInfo->AgentActor.Get() : nullptr; }
	AActor* GetOwnerActor() const { return AgentInfo.IsValid() ? AgentInfo->OwnerActor.Get() : nullptr; }


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

	bool CanTriggerAction(FNexusActionDef* ActionDef, const FNexusEventMessage& EventMessage);

	void CallOrAddSetupCompletedDelegate(FOnNexusActionComponentSetupCompletedSignature::FDelegate&& Delegate);
	void RemoveSetupCompletedDelegate(const void* Object);

	void AddSetupCompletedDelegate(FOnNexusActionComponentSetupCompletedSignature::FDelegate&& Delegate);

	bool IsSetupCompleted() const { return bSetupCompleted; }

private:
	template <typename T>
	void EnsureSubComponent()
	{
		for (const auto& Comp : SubComponents)
		{
			if (Comp->IsA<T>())
			{
				return;
			}
		}
		
		if (T* Existing = GetOwner()->FindComponentByClass<T>())
		{
			SubComponents.Add(Existing);
		}
		else
		{
			T* Comp = NewObject<T>(GetOwner());
			Comp->RegisterComponent();
			SubComponents.Add(Comp);
		}
	}

	template <typename T>
	T* GetCachedComponent(TObjectPtr<T>& Cache) const
	{
		if (!Cache)
		{
			Cache = Cast<T>(GetOwner()->GetComponentByClass(T::StaticClass()));
		}
		return Cache;
	}


	void InternalOnShowDebugInfo(AActor* DebugTarget, AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y);

	void InternalSetupActionComponent(AActor* InAgentActor);

	bool HasActionTriggerAuthority(UNexusAction* Action) const;

	void OnActionDefAdded(FNexusActionDef& ActionDef);
	void OnActionDefRemoved(FNexusActionDef& ActionDef);


	void HandleTriggerableActionOnAdded(const FNexusActionDef& NewActionDef);
	void HandleTriggerableActionOnRemoved(const FNexusActionDef& ActionDef);
	void BindActionTriggerEvent(const FNexusActionDef& NewActionDef);
	void UnbindActionTriggerEvent(const FNexusActionDef& ActionDef);
	void OnActionEventTriggered(FGameplayTag GameplayTag, const FNexusEventMessage& EventMessage, FNexusActionDefHandle ActionDefHandle);
	void OnActionEnded(FNexusActionDefHandle ActionDefHandle, UNexusAction* Action);


	bool InternalCanTriggerAction(const FNexusActionDef& ActionDef) const;
	void LocalTriggerAction(FNexusActionDef* ActionDef);
	UFUNCTION(Reliable, Server)
	void ServerTryTriggerAction(FNexusActionDefHandle ActionDefHandle, const FNexusEventMessageReplicated& EventMessageReplicated, FNexusPredictionTag PredictionTag);
	UFUNCTION(Reliable, Client)
	void ClientTriggerAction(FNexusActionDefHandle ActionDefHandle, const FNexusEventMessageReplicated& EventMessageReplicated, FNexusPredictionTag PredictionTag);
	UFUNCTION(Reliable, Client)
	void ClientTriggerActionRequestSucceeded(FNexusActionDefHandle ActionDefHandle, FNexusPredictionTag PredictionTag);
	UFUNCTION(Reliable, Client)
	void ClientTriggerActionRequestFailed(FNexusActionDefHandle ActionDefHandle, FNexusPredictionTag PredictionTag);


	UFUNCTION(Reliable, Server)
	void ServerRemoteRequestTryTriggerAction(FNexusActionDefHandle ActionDefHandle, const FNexusEventMessage& EventMessage);
	UFUNCTION(Reliable, Client)
	void ClientRemoteRequestTryTriggerAction(FNexusActionDefHandle ActionDefHandle, const FNexusEventMessage& EventMessage);
	FNexusPredictionTag GetCurrentPredictionTag() const;

protected:
	UNexusCueComponent* GetCueComponent() const;
	UNexusPredictionComponent* GetPredictionComponent() const;
	UNexusSideEffectComponent* GetSideEffectComponent() const;
	UNexusPropertyComponent* GetPropertyComponent() const;
	UNexusGameplayTagComponent* GetGameplayTagComponent() const;

private:
	UPROPERTY()
	TWeakObjectPtr<UNexusEventManagerComponent> EventManagerComponent;


	bool bSetupCompleted = false;
	FOnNexusActionComponentSetupCompletedSignature OnActionComponentSetupCompletedDelegate;
	FOnNexusActionComponentTeardownCompletedSignature OnActionComponentTeardownCompletedDelegate;


	int32 ActionScopeLockCount = 0;

	struct FNexusPendingAddActionInfo
	{
		FNexusActionDef ActionDef;
		bool bIsPendingTrigger = false;
		FNexusEventMessage PendingEventMessage;
	};

	TArray<FNexusPendingAddActionInfo> ActionPendingAdds;
	TArray<FNexusActionDefHandle> ActionPendingRemoves;

	struct FNexusPendingActionTriggerRequest
	{
		FNexusActionDefHandle ActionDefHandle;
		FNexusEventMessage EventMessage;
	};

	TArray<FNexusPendingActionTriggerRequest> ClientPendingActionTriggerRequests;


	TSharedPtr<FNexusAgentInfo> AgentInfo;

	TMap<FNexusActionDefHandle, TArray<FNexusEventCallbackHandle>> BoundedActionEventHandles;

	UPROPERTY(Replicated)
	FNexusActionDefContainer ActionDefs;


	TMap<FNexusActionDefHandle, FNexusSideEffectDefHandle> TagSideEffectMap;


	void LocalOnTriggerActionConfirmed(FNexusActionDefHandle ActionDefHandle, FNexusPredictionTag PredictionTag);

public:
	void PushDynamicTag(const FGameplayTag& Tag);
	void PopDynamicTag(const FGameplayTag& Tag);

private:
	UPROPERTY()
	mutable TObjectPtr<UNexusPredictionComponent> PredictionComponentCached;
	UPROPERTY()
	mutable TObjectPtr<UNexusSideEffectComponent> SideEffectComponentCached;
	UPROPERTY()
	mutable TObjectPtr<UNexusPropertyComponent> PropertyComponentCached;
	UPROPERTY()
	mutable TObjectPtr<UNexusGameplayTagComponent> GameplayTagComponentCached;
	UPROPERTY()
	mutable TObjectPtr<UNexusCueComponent> CueComponentCached;
	UPROPERTY()
	TArray<TObjectPtr<UNexusAgentBoundComponent>> SubComponents;
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
