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
	static void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static UNexusActionComponent* GetActionComponentFromActor(AActor* Actor);

	void SetupActionComponent(AActor* InAgentActor);
	void TeardownActionComponent();


	virtual void OnSetupActionComponent()
	{
	}

	void CallOrAddSetupCompletedDelegate(FOnNexusActionComponentSetupCompletedSignature::FDelegate&& Delegate);
	void RemoveSetupCompletedDelegate(const void* Object);
	void AddSetupCompletedDelegate(FOnNexusActionComponentSetupCompletedSignature::FDelegate&& Delegate);

	bool IsSetupCompleted() const { return bSetupCompleted; }

private:
	void InternalSetupActionComponent(AActor* InAgentActor);
	void InternalOnShowDebugInfo(AActor* DebugTarget, AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y);

public:
	// ------------------------------------------------------------------------------
	// AgentInfo
	// ------------------------------------------------------------------------------
	AActor* GetAgentActor() const { return AgentInfo->AgentActor.Get(); }
	AActor* GetOwnerActor() const { return AgentInfo->OwnerActor.Get(); }
	bool IsAgentLocallyControlled() const { return AgentInfo->IsLocallyControlled(); }
	bool IsAgentLocallyPlayerControlled() const { return AgentInfo->IsLocallyPlayerControlled(); }
	bool IsOwnerActorAuthoritative() const { return AgentInfo->IsOwnerActorAuthoritative(); }
	TWeakPtr<FNexusAgentInfo> GetAgentInfo() const { return AgentInfo; }


	// ------------------------------------------------------------------------------
	// Action Add/Remove
	// ------------------------------------------------------------------------------

	UFUNCTION(BlueprintCallable)
	FNexusActionDefHandle AuthAddAction(TSubclassOf<UNexusAction> ActionClass, UObject* SourceObject = nullptr);
	void AuthRemoveAction(const FNexusActionDefHandle& ActionDefHandle);
	void AuthRemoveAllActions();

	void IncreaseActionListLock();
	void DecreaseActionListLock();

private:
	FNexusActionDefHandle InternalAuthAddAction(const FNexusActionDef& ActionDef);
	void OnActionDefAdded(const FNexusActionDef& ActionDef);
	void OnActionDefRemoved(const FNexusActionDef& ActionDef);
	void HandleTriggerableActionOnAdded(const FNexusActionDef& NewActionDef, UNexusAction* ActionInstance);
	void HandleTriggerableActionOnRemoved(const FNexusActionDefHandle& Handle);
	void BindActionTriggerEvent(const FNexusActionDef& NewActionDef, UNexusAction* ActionInstance);
	void UnbindActionTriggerEvent(const FNexusActionDefHandle& Handle);

	// ------------------------------------------------------------------------------
	// Action Trigger
	// ------------------------------------------------------------------------------
public:
	bool HasActionTriggerAuthority(UNexusAction* Action) const;
	bool CanTriggerAction(UNexusAction* ActionInstance, const FNexusEventMessage& EventMessage);
	UFUNCTION(BlueprintCallable)
	void TryTriggerAction(FNexusActionDefHandle ActionDefHandle, const FNexusEventMessage& EventMessage = FNexusEventMessage());

private:
	bool InternalCanTriggerAction(UNexusAction* ActionInstance) const;
	void LocalTriggerAction(const FNexusActionDefHandle& ActionDefHandle, UNexusAction* ActionInstance);
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

	void LocalOnTriggerActionConfirmed(FNexusActionDefHandle ActionDefHandle, FNexusPredictionTag PredictionTag);

	void OnActionEventTriggered(FGameplayTag GameplayTag, const FNexusEventMessage& EventMessage, FNexusActionDefHandle ActionDefHandle);
	void OnActionEnded(FNexusActionDefHandle ActionDefHandle, UNexusAction* Action);

public:
	UFUNCTION(BlueprintCallable)
	FNexusActionDefHandle FindActionDefHandle(TSubclassOf<UNexusAction> ActionClass, UObject* SourceObject) const;
	FNexusActionDef* FindActionDefByHandle(FNexusActionDefHandle Handle);

	UNexusAction* CreateActionInstance(const FNexusActionDef& Def);
	void DestroyActionInstance(FNexusActionDefHandle Handle);
	UNexusAction* FindActionInstanceByHandle(FNexusActionDefHandle Handle);
	const UNexusAction* FindActionInstanceByHandle(FNexusActionDefHandle Handle) const;
	const FNexusActionDefContainer& GetActionDefs() const { return ActionDefs; }
	FNexusPredictionTag GetCurrentPredictionTag() const;
	
	
	// ------------------------------------------------------------------------------
	// SideEffect
	// ------------------------------------------------------------------------------
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Trigger Side Effect"))
	static void BP_TriggerSideEffectToActor(UNexusAction* Action, AActor* SideEffectTarget, TSubclassOf<UNexusSideEffect> SideEffectClass);
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Trigger Side Effect By Def"))
	static void BP_TriggerSideEffectToActorByDef(UNexusAction* Action, AActor* SideEffectTarget, const FNexusSideEffectDef& SideEffectDef);
	UFUNCTION(BlueprintCallable)
	static FNexusSideEffectDef MakeSideEffectDef(UNexusAction* Action, TSubclassOf<UNexusSideEffect> SideEffectClass);
	void TriggerSideEffect(TSubclassOf<UNexusSideEffect> SideEffectClass, UNexusAction* Action);
	void TriggerSideEffectByDef(const FNexusSideEffectDef& NewSideEffectDef, UNexusAction* Action, FNexusPredictionEventSignature::FDelegate&& OnPredictionEnded = {}, FNexusPredictionEventSignature::FDelegate&& OnPredictionFailed = {}) const;


	// ------------------------------------------------------------------------------
	// Property
	// ------------------------------------------------------------------------------
	UFUNCTION(BlueprintCallable)
	UNexusProperty* GetProperty(FGameplayTag Tag);
	void AddProperty(FGameplayTag Tag, float Value);
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static UNexusProperty* GetPropertyFromActor(AActor* Actor, FGameplayTag Tag);
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static float GetPropertyValueFromActor(AActor* Actor, FGameplayTag Tag);

	void AddStaticOperation(FGameplayTag Tag, FNexusPropertyOperation Operation);
	void AddDynamicOperation(FGameplayTag Tag, FNexusPropertyOperation Operation);
	void RemoveOperationByHandle(FGameplayTag Tag, const FNexusPropertyOperationHandle& OperationHandle);


	// ------------------------------------------------------------------------------
	// GameplayTag
	// ------------------------------------------------------------------------------
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	void PushDynamicTag(const FGameplayTag& Tag);
	void PopDynamicTag(const FGameplayTag& Tag);


	// ------------------------------------------------------------------------------
	// Cue
	// ------------------------------------------------------------------------------
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Trigger Cue"))
	static void BP_TriggerCue(UNexusAction* Action, TSubclassOf<ANexusCue> CueClass, const FNexusTargetDataHandle& TargetDataHandle);
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, meta = (DisplayName = "Auth End Cue"))
	static void BP_AuthEndCue(UNexusAction* Action, FNexusLoopingCueHandle CueHandle);
	void SimTriggerCue(const FNexusTriggerCueParams& CueParams, FNexusLoopingCueHandle CueHandle);


	// ------------------------------------------------------------------------------
	// Event
	// ------------------------------------------------------------------------------
	void HandleEvent(FGameplayTag EventTag, const void* Message, UScriptStruct* MessageType);

	template <typename FMessageStruct>
	static void SendEventToActor(FGameplayTag EventTag, const FMessageStruct& Message, AActor* TargetActor)
	{
		if (UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(TargetActor))
		{
			ActionComponent->HandleEvent(EventTag, &Message, TBaseStructure<FMessageStruct>::Get());
		}
	}

	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Nexus|Event", meta = (CustomStructureParam = "Message", DisplayName= "Send Event To Actor"))
	static void BP_SendEventToActor(FGameplayTag EventTag, AActor* TargetActor, const int32& Message);
	DECLARE_FUNCTION(execBP_SendEventToActor);

	template <typename FMessageStruct, typename TOwner, typename... VarType>
	FNexusEventCallbackHandle BindEventCallback(FGameplayTag EventTag, TOwner* Object, void (TOwner::*Function)(FGameplayTag, const FMessageStruct&, VarType...), VarType... Vars);

	FNexusEventCallbackHandle BindEventCallbackDirect(FGameplayTag EventTag, TFunction<void(FGameplayTag, const void*)>&& Callbacks, UScriptStruct* MessageType) const;
	void UnbindEventCallback(FNexusEventCallbackHandle Handle) const;

private:
	template <typename T>
	T* GetCachedComponent(TObjectPtr<T>& Cache) const;

protected:
	UNexusCueComponent* GetCueComponent() const;
	UNexusPredictionComponent* GetPredictionComponent() const;
	UNexusSideEffectComponent* GetSideEffectComponent() const;
	UNexusPropertyComponent* GetPropertyComponent() const;
	UNexusGameplayTagComponent* GetGameplayTagComponent() const;
	UNexusEventManagerComponent* GetEventManagerComponent() const;

private:
	bool bSetupCompleted = false;
	FOnNexusActionComponentSetupCompletedSignature OnActionComponentSetupCompletedDelegate;
	FOnNexusActionComponentTeardownCompletedSignature OnActionComponentTeardownCompletedDelegate;

	TSharedPtr<FNexusAgentInfo> AgentInfo;

	UPROPERTY(Replicated)
	FNexusActionDefContainer ActionDefs;
	UPROPERTY()
	TMap<FNexusActionDefHandle, TObjectPtr<UNexusAction>> LocalActionInstanceMap;


	int32 ActionScopeLockCount = 0;
	TArray<FNexusActionDefHandle> ActionPendingRemoves;

	struct FNexusPendingAddActionInfo
	{
		FNexusActionDef ActionDef;
		bool bIsPendingTrigger = false;
		FNexusEventMessage PendingEventMessage;
	};

	TArray<struct FNexusPendingAddActionInfo> ActionPendingAdds;

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
	mutable TObjectPtr<UNexusEventManagerComponent> EventManagerComponentCached;

	TMap<FNexusActionDefHandle, FNexusSideEffectDefHandle> TagSideEffectMap;


	struct FNexusPendingActionTriggerRequest
	{
		FNexusActionDefHandle ActionDefHandle;
		FNexusEventMessage EventMessage;
	};

	TArray<FNexusPendingActionTriggerRequest> ClientPendingActionTriggerRequests;
	TMap<FNexusActionDefHandle, TArray<FNexusEventCallbackHandle>> BoundedActionEventHandles;
};


template <typename FMessageStruct, typename TOwner, typename... VarType>
FNexusEventCallbackHandle UNexusActionComponent::BindEventCallback(FGameplayTag EventTag, TOwner* Object, void (TOwner::*Function)(FGameplayTag, const FMessageStruct&, VarType...), VarType... Vars)
{
	return GetEventManagerComponent()->BindEventCallback<FMessageStruct>(EventTag, Object, Function, Vars...);
}



template <typename T>
T* UNexusActionComponent::GetCachedComponent(TObjectPtr<T>& Cache) const
{
	if (!Cache)
	{
		Cache = Cast<T>(GetOwner()->GetComponentByClass(T::StaticClass()));
	}
	return Cache;
}
