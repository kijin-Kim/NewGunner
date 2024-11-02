// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GunnerActionAgentInfo.h"
#include "GunnerActionDefinition.h"
#include "GunnerActionSideEffectDefinition.h"
#include "AsyncAction/GunnerActionNetPrediction.h"
#include "Components/ActorComponent.h"
#include "Gunner/Core/Event/GunnerEventManagerComponent.h"
#include "Gunner/Core/Input/GunnerEventMessage.h"
#include "GunnerActionComponent.generated.h"


class UGunnerActionSideEffect;

USTRUCT()
struct FGunnerLocalActionTriggerState
{
	GENERATED_BODY()

	bool operator==(const FGunnerLocalActionTriggerState& ClientActionTriggerState) const = default;

	UPROPERTY()
	FGunnerActionDefinitionHandle ActionDefinitionHandle;
	UPROPERTY()
	int32 ActionTriggerID = 0;
	UPROPERTY()
	bool bIsTriggering = false;
	
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
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	
	FGunnerActionDefinitionHandle AuthAddAction(const FGunnerActionDefinition& ActionDefinition);
	void AuthRemoveAction(const FGunnerActionDefinitionHandle& ActionDefinitionHandle);
	void AuthRemoveAllActions();
	void TryTriggerAction(FGunnerActionDefinitionHandle ActionDefinitionHandle, const FGunnerEventMessage& EventMessage);

	void IncrementActionListLock();
	void DecrementActionListLock();

	UFUNCTION(BlueprintCallable)
	static UGunnerActionComponent* GetActionComponentFromActor(AActor* Actor);

	bool HasActionTriggerAuthority(UGunnerAction* Action) const;

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Trigger Side Effect To Actor"))
	static void BP_TriggerSideEffectToActor(UGunnerAction* Action, AActor* SideEffectTarget, TSubclassOf<UGunnerActionSideEffect> SideEffectClass);
	void TriggerSideEffect(TSubclassOf<UGunnerActionSideEffect> SideEffectClass, FGunnerActionNetPredictionHandle PredictionHandle);

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
	void ServerTryTriggerAction(FGunnerActionDefinitionHandle ActionDefinitionHandle, const FGunnerEventMessageReplicated& EventMessageReplicated, const TArray<FGunnerLocalActionTriggerState>& ClientActionTriggerStates, FGunnerActionNetPredictionHandle PredictionHandle);
	UFUNCTION(Reliable, Client)
	void ClientTriggerAction(FGunnerActionDefinitionHandle ActionDefinitionHandle, const FGunnerEventMessageReplicated& EventMessageReplicated);
	UFUNCTION(Reliable, Client)
	void ClientTriggerActionRequestSucceeded(FGunnerActionDefinitionHandle ActionDefinitionHandle, FGunnerActionNetPredictionHandle PredictionHandle);
	UFUNCTION(Reliable, Client)
	void ClientTriggerActionRequestFailed(FGunnerActionDefinitionHandle ActionDefinitionHandle, FGunnerActionNetPredictionHandle PredictionHandle);
	
	

	void AggregateActionTriggerStates(TArray<FGunnerLocalActionTriggerState>& OutActionTriggerStates);

private:
	UPROPERTY(ReplicatedUsing=OnRep_ActionDefinitions)
	TArray<FGunnerActionDefinition> ActionDefinitions;
	int32 ActionScopeLockCount = 0;
	TArray<FGunnerActionDefinition> ActionPendingAdds;
	TArray<FGunnerActionDefinitionHandle> ActionPendingRemoves;

	TSharedPtr<FGunnerActionAgentInfo> AgentInfo;

	FGameplayTagContainer OwnedTags;

	TMap<FGunnerActionDefinitionHandle, TArray<FGunnerEventCallbackHandle>> BoundedActionEventHandles;
	

	
	struct FGunnerNetTriggerDelayedAction
	{
		FGunnerActionDefinitionHandle ActionDefinitionHandle;
		FGunnerEventMessage EventMessage;
		double TriggerTime;
	};
	
	TArray<FGunnerNetTriggerDelayedAction> NetTriggerDelayedActions;

	
	TArray<FGunnerActionSideEffectDefinition> SideEffectDefinitions;

	TArray<FGunnerActionNetPrediction> NetPredictions;

	UFUNCTION()
	void OnRep_NetPredictionHandles();
	
	UPROPERTY(ReplicatedUsing=OnRep_NetPredictionHandles)
	TArray<FGunnerActionNetPredictionHandle> NetPredictionHandles;
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
