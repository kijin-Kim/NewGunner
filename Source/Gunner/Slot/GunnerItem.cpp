// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerItem.h"

#include "Action/NexusAction.h"
#include "Action/NexusActionComponent.h"
#include "Engine/Canvas.h"
#include "Gunner/Gunner.h"

AGunnerItem::AGunnerItem()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

bool AGunnerItem::CanAcquire(const TArray<AGunnerItem*>& InventoryItems) const
{
	 return true;
}

void AGunnerItem::OnAcquired(AActor* InAgentActor)
{
	AgentActor = InAgentActor;
	GR_VLOG(InAgentActor, LogGunner, Log, TEXT("아이템 [%s] 획득"), *GetName());
	if (HasAuthority())
	{
		AuthAddDesiredActions(PersistentActivationActions, PersistentActivationActionHandles);
	}
}

void AGunnerItem::OnRemoved()
{
	GR_VLOG(AgentActor, LogGunner, Log, TEXT("아이템 [%s] 제거"), *GetName());
	if (HasAuthority())
	{
		AuthRemoveDesiredActions(PersistentActivationActionHandles);
	}
	
	AgentActor = nullptr;
}

void AGunnerItem::AuthAddDesiredActions(const TArray<TSubclassOf<UNexusAction>>& ActionsToAdd, TArray<FNexusActionDefHandle>& AddedActionHandles)
{
	check(HasAuthority());
	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(AgentActor);
	check(ActionComponent);
	if (ActionComponent)
	{
		for (auto ActionClass : ActionsToAdd)
		{
			if (ActionClass)
			{
				FNexusActionDefHandle AddedHandle = ActionComponent->AuthAddAction(ActionClass, this);
				AddedActionHandles.Add(AddedHandle);
			}
		}
	}
}

void AGunnerItem::AuthRemoveDesiredActions(TArray<FNexusActionDefHandle>& AddedActionHandles)
{
	check(HasAuthority())
	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(AgentActor);
	check(ActionComponent);
	if (ActionComponent)
	{
		for (auto& ActionHandle : AddedActionHandles)
		{
			ActionComponent->AuthRemoveAction(ActionHandle);
		}
		AddedActionHandles.Empty();
	}
}
