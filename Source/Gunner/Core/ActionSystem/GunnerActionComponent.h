// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GunnerActionAgentInfo.h"
#include "GunnerActionDefinition.h"
#include "Components/ActorComponent.h"
#include "Gunner/Core/Input/GunnerEventMessage.h"
#include "GunnerActionComponent.generated.h"


class UGunnerActionSet;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UGunnerActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGunnerActionComponent();
	void InitActionComponent(AActor* InOwnerActor, AActor* InAgentActor);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FGunnerActionDefinitionHandle AddAction(const FGunnerActionDefinition& ActionDefinition);
	void RemoveAction(const FGunnerActionDefinitionHandle& ActionDefinitionHandle);
	void TryTriggerAction(FGunnerActionDefinitionHandle ActionDefinitionHandle, const FGunnerEventMessage& EventMessage);

	void IncrementActionListLock();
	void DecrementActionListLock();

	static UGunnerActionComponent* GetActionComponentFromActor(AActor* Actor);

private:
	static void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Arg);
	void InternalOnShowDebugInfo(AActor* DebugTarget, AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Arg);
	
	void BindActionTriggerEvent(const FGunnerActionDefinition& NewActionDefinition);
	void OnActionEventTriggered(FGameplayTag GameplayTag, const FGunnerEventMessage& EventMessage, FGunnerActionDefinitionHandle ActionDefinitionHandle);
	UFUNCTION()
	void OnRep_ActionDefinitions(const TArray<FGunnerActionDefinition>& OldActionDefinitions);
	void OnActionEnded(FGunnerActionDefinitionHandle ActionDefinitionHandle, UGunnerAction* Action);
	FGunnerActionDefinition* FindActionDefinitionByHandle(FGunnerActionDefinitionHandle ActionDefinitionHandle);

	bool CanTriggerAction(const FGunnerActionDefinition& ActionDefinition) const;
	void LocalTriggerAction(FGunnerActionDefinition* ActionDefinition, FGunnerActionDefinitionHandle ActionDefinitionHandle, const FGunnerEventMessage& EventMessage);
	UFUNCTION(Reliable, Server)
	void ServerTryTriggerAction(FGunnerActionDefinitionHandle ActionDefinitionHandle, const FGunnerEventMessage& EventMessage);
	UFUNCTION(Reliable, Client)
	void ClientTriggerAction(FGunnerActionDefinitionHandle ActionDefinitionHandle, const FGunnerEventMessage& EventMessage);

private:
	UPROPERTY(ReplicatedUsing=OnRep_ActionDefinitions)
	TArray<FGunnerActionDefinition> ActionDefinitions;
	int32 ActionScopeLockCount = 0;
	TArray<FGunnerActionDefinition> ActionPendingAdds;
	TArray<FGunnerActionDefinitionHandle> ActionPendingRemoves;

	TSharedPtr<FGunnerActionAgentInfo> AgentInfo;

	FGameplayTagContainer OwnedTags;
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
