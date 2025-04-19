// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerSlotItem.h"

#include "Action/NexusAction.h"
#include "Action/NexusActionComponent.h"
#include "Engine/Canvas.h"
#include "Gunner/Gunner.h"


AGunnerSlotItem::AGunnerSlotItem()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AGunnerSlotItem::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y)
{
	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;
	DisplayDebugManager.DrawString(FString::Printf(TEXT("슬롯 아이템: %s"), *UEnum::GetValueAsString(GetSlotType())));
}

void AGunnerSlotItem::OnAcquired(AActor* AgentActor)
{
	GR_VLOG(AgentActor, LogGunner, Log, TEXT("아이템 [%s] 획득"), *GetName());
	if (HasAuthority())
	{
		AuthAddDesiredActions(AgentActor, PersistentActivationActions, PersistentActivationActionHandles);
	}
}

void AGunnerSlotItem::OnRemoved(AActor* AgentActor)
{
	GR_VLOG(AgentActor, LogGunner, Log, TEXT("아이템 [%s] 제거"), *GetName());
	if (HasAuthority())
	{
		AuthRemoveDesiredActions(AgentActor, PersistentActivationActionHandles);
	}
}

void AGunnerSlotItem::OnActivated(AActor* AgentActor)
{
	GR_VLOG(AgentActor, LogGunner, Log, TEXT("아이템 [%s] 활성화"), *GetName());
	if (HasAuthority())
	{
		AuthAddDesiredActions(AgentActor, TransientActivationActions, TransientActivationActionHandles);
	}
}

void AGunnerSlotItem::OnDeactivated(AActor* AgentActor)
{
	GR_VLOG(AgentActor, LogGunner, Log, TEXT("아이템 [%s] 비활성화"), *GetName());
	if (HasAuthority())
	{
		AuthRemoveDesiredActions(AgentActor, TransientActivationActionHandles);
	}
}


UNexusActionComponent* AGunnerSlotItem::GetActionComponent(AActor* AgentActor) const
{
	return AgentActor ? UNexusActionComponent::GetActionComponentFromActor(AgentActor) : nullptr;
}


void AGunnerSlotItem::AuthAddDesiredActions(AActor* AgentActor, const TArray<TSubclassOf<UNexusAction>>& ActionsToAdd, TArray<FNexusActionDefHandle>& AddedActionHandles)
{
	check(HasAuthority());
	UNexusActionComponent* ActionComponent = GetActionComponent(AgentActor);
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

void AGunnerSlotItem::AuthRemoveDesiredActions(AActor* AgentActor, TArray<FNexusActionDefHandle>& AddedActionHandles)
{
	check(HasAuthority())
	UNexusActionComponent* ActionComponent = GetActionComponent(AgentActor);
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
