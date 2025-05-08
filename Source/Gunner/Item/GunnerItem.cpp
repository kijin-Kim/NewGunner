// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerItem.h"

#include "GunnerItemDef.h"
#include "Action/NexusAction.h"
#include "Action/NexusActionComponent.h"
#include "Engine/Canvas.h"
#include "Gunner/Gunner.h"
#include "Gunner/_Core/GunnerBlueprintFunctionLibrary.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"
#include "Net/UnrealNetwork.h"

AGunnerItem::AGunnerItem()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AGunnerItem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	OnRemoved();
	Super::EndPlay(EndPlayReason);
}

void AGunnerItem::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGunnerItem, ItemDef);
}

void AGunnerItem::InitializeItem(const UGunnerItemDef* InItemDef)
{
	ItemDef = InItemDef;
}

bool AGunnerItem::CanAcquire(const TArray<AGunnerItem*>& InventoryItems) const
{
	return true;
}

void AGunnerItem::OnAcquired(AActor* InAgentActor)
{
	AgentActor = InAgentActor;
	if (HasAuthority() && ItemDef)
	{
		UGunnerBlueprintFunctionLibrary::AuthAddDesiredActions(AgentActor, this, ItemDef->AcquiredActionClasses, PersistentActivationActionHandles);
	}
}

void AGunnerItem::OnRemoved()
{
	if (HasAuthority())
	{
		UGunnerBlueprintFunctionLibrary::AuthRemoveDesiredActions(AgentActor, PersistentActivationActionHandles);
		PersistentActivationActionHandles.Empty();
	}

	AgentActor = nullptr;
}
