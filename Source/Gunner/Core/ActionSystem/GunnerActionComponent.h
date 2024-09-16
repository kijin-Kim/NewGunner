// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerActionAgentInfo.h"
#include "GunnerActionDefinition.h"
#include "Components/ActorComponent.h"
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
	void TryTriggerAction(FGunnerActionDefinitionHandle ActionDefinitionHandle);


	void TEST_TRIGGER_ACTIONS();

	void IncrementActionListLock();
	void DecrementActionListLock();
	
private:
	UFUNCTION()
	void OnRep_ActionDefinitions();
	void OnActionEnded(FGunnerActionDefinitionHandle ActionDefinitionHandle, UGunnerAction* Action);
	FGunnerActionDefinition* FindActionDefinitionByHandle(FGunnerActionDefinitionHandle ActionDefinitionHandle);
	
	void LocalTriggerAction(FGunnerActionDefinition* ActionDefinition, FGunnerActionDefinitionHandle ActionDefinitionHandle);
	UFUNCTION(Reliable, Server)
	void ServerTryTriggerAction(FGunnerActionDefinitionHandle ActionDefinitionHandle);
	UFUNCTION(Reliable, Client)
	void ClientTriggerAction(FGunnerActionDefinitionHandle ActionDefinitionHandle);
	
	
private:
	UPROPERTY(ReplicatedUsing=OnRep_ActionDefinitions)
	TArray<FGunnerActionDefinition> ActionDefinitions;
	int32 ActionScopeLockCount = 0;
	TArray<FGunnerActionDefinition> ActionPendingAdds;
	TArray<FGunnerActionDefinitionHandle> ActionPendingRemoves;
	

	TSharedPtr<FGunnerActionAgentInfo> AgentInfo;
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
