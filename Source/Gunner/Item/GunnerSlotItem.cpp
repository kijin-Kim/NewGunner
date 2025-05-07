// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerSlotItem.h"

#include "GunnerItemDef.h"
#include "Action/NexusAction.h"
#include "Action/NexusActionComponent.h"
#include "Engine/Canvas.h"
#include "Gunner/Gunner.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"


void AGunnerSlotItem::CollectDisplayDebugString(TArray<FGunnerInventoryDisplayDebugString>& OutDisplayDebugstrings) const
{
	Super::CollectDisplayDebugString(OutDisplayDebugstrings);

	if (OutDisplayDebugstrings.Num() < static_cast<int32>(EGunnerSlotType::Num))
	{
		UEnum* SlotTypeEnum = StaticEnum<EGunnerSlotType>();
		OutDisplayDebugstrings.SetNum(static_cast<int32>(EGunnerSlotType::Num));
		for (int32 i = 0; i < static_cast<int32>(EGunnerSlotType::Num); i++)
		{
			OutDisplayDebugstrings[i].Color = FColor{255, 255, 255, 127};
			
			OutDisplayDebugstrings[i].DisplayDebugString = FString::Printf(TEXT("%s (None)"), *SlotTypeEnum->GetDisplayNameTextByIndex(i).ToString());
		}
	}
	
	OutDisplayDebugstrings[static_cast<int32>(SlotType)].DisplayDebugString = FString::Printf(TEXT("%s (%s)"), *UEnum::GetDisplayValueAsText(SlotType).ToString(), *GetName());
	UNexusProperty* SlotIndexProperty = GetSlotIndexProperty();
	OutDisplayDebugstrings[static_cast<int32>(SlotType)].Color = SlotIndexProperty && static_cast<EGunnerSlotType>(SlotIndexProperty->GetDynamicValue()) == SlotType ? FColor::Orange : FColor{255, 255, 255, 127};
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
	if (!SlotIndexProperty)
	{
		return;
	}

	SlotIndexProperty->OnDirtyDelegate.RemoveDynamic(this, &ThisClass::HandleSlotIndexDirty);
	if (SlotType == static_cast<EGunnerSlotType>(SlotIndexProperty->GetDynamicValue()))
	{
		OnDeactivated();
	}
	Super::OnRemoved();
}


void AGunnerSlotItem::OnActivated()
{
	GR_VLOG(AgentActor, LogGunnerInventory, Display, TEXT("아이템 활성화: %s"), *ToString());
}

void AGunnerSlotItem::OnDeactivated()
{
	GR_VLOG(AgentActor, LogGunnerInventory, Display, TEXT("아이템 비활성화: %s"), *ToString());
}


void AGunnerSlotItem::HandleSlotIndexDirty(float OldValue, float NewValue)
{
	if (OldValue == NewValue)
	{
		return;
	}

	if (SlotType == static_cast<EGunnerSlotType>(OldValue))
	{
		OnDeactivated();
	}

	if (SlotType == static_cast<EGunnerSlotType>(NewValue))
	{
		OnActivated();
	}
}

UNexusProperty* AGunnerSlotItem::GetSlotIndexProperty() const
{
	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(AgentActor);
	if (!ActionComponent)
	{
		return nullptr;
	}
	return ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_SlotIndex);
}
