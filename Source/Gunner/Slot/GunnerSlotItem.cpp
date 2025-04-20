// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerSlotItem.h"

#include "Action/NexusAction.h"
#include "Action/NexusActionComponent.h"
#include "Engine/Canvas.h"
#include "Gunner/Gunner.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"


void AGunnerSlotItem::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y)
{
	Super::OnShowDebugInfo(HUD, Canvas, DebugDisplayInfo, X, Y);
	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;
	DisplayDebugManager.DrawString(FString::Printf(TEXT("슬롯 아이템: %s"), *UEnum::GetValueAsString(GetSlotType())));
}

bool AGunnerSlotItem::CanAcquire(const TArray<AGunnerItem*>& InventoryItems) const
{
	bool bResult = Super::CanAcquire(InventoryItems);
	if (!bResult)
	{
		return false;
	}

	for (AGunnerItem* InventoryItem : InventoryItems)
	{
		AGunnerSlotItem* SlotItem = Cast<AGunnerSlotItem>(InventoryItem);
		if (!SlotItem)
		{
			continue;
		}

		if (SlotItem->GetSlotType() == SlotType)
		{
			return false;
		}
	}
	return true;
}

void AGunnerSlotItem::PostOnAcquired()
{
	Super::PostOnAcquired();
	UNexusProperty* SlotIndexProperty = GetSlotIndexProperty();
	check(SlotIndexProperty);
	SlotIndexProperty->OnDirtyDelegate.AddDynamic(this, &ThisClass::HandleSlotIndexDirty);
	if (SlotType == static_cast<EGunnerSlotType>(SlotIndexProperty->GetDynamicValue()))
	{
		OnActivated();
	}
}

void AGunnerSlotItem::OnRemoved()
{
	UNexusProperty* SlotIndexProperty = GetSlotIndexProperty();
	SlotIndexProperty->OnDirtyDelegate.RemoveDynamic(this, &ThisClass::HandleSlotIndexDirty);
	if (SlotType == static_cast<EGunnerSlotType>(SlotIndexProperty->GetDynamicValue()))
	{
		OnDeactivated();
	}
	Super::OnRemoved();
}

void AGunnerSlotItem::OnActivated()
{
	GR_VLOG(AgentActor, LogGunner, Log, TEXT("아이템 [%s] 활성화"), *GetName());
	if (HasAuthority())
	{
		AuthAddDesiredActions(TransientActivationActions, TransientActivationActionHandles);
	}
}

void AGunnerSlotItem::OnDeactivated()
{
	GR_VLOG(AgentActor, LogGunner, Log, TEXT("아이템 [%s] 비활성화"), *GetName());
	if (HasAuthority())
	{
		AuthRemoveDesiredActions(TransientActivationActionHandles);
	}
}

void AGunnerSlotItem::HandleSlotIndexDirty(float OldValue, float NewValue)
{
	if (OldValue == NewValue)
	{
		return;
	}

	if (SlotType == static_cast<EGunnerSlotType>(NewValue))
	{
		OnActivated();
	}


	if (SlotType == static_cast<EGunnerSlotType>(OldValue))
	{
		OnDeactivated();
	}
}

UNexusProperty* AGunnerSlotItem::GetSlotIndexProperty() const
{
	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(AgentActor);
	check(ActionComponent);
	return ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_SlotIndex);
}
