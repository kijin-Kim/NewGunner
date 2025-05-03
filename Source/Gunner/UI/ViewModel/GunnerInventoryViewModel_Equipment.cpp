// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerInventoryViewModel_Equipment.h"

#include "Action/NexusActionComponent.h"
#include "Gunner/Item/GunnerInventoryManagerComponent.h"
#include "Gunner/Item/GunnerItemDef.h"
#include "Gunner/Item/GunnerSlotItem.h"
#include "Gunner/Item/Part/GunnerItemPart_Icon.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"


void UGunnerInventoryViewModel_Equipment::OnCreateViewModel(const UUserWidget* UserWidget)
{
	Super::OnCreateViewModel(UserWidget);

	const TArray<AGunnerItem*>& Items = InventoryManagerComponent->GetInventoryItems();

	for (AGunnerItem* Item : Items)
	{
		if (AGunnerSlotItem* SlotItem = Cast<AGunnerSlotItem>(Item))
		{
			OnItemAcquired(SlotItem);
		}
	}

	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(EquipmentHudData);

	InventoryManagerComponent->OnItemAcquiredDelegate.AddDynamic(this, &UGunnerInventoryViewModel_Equipment::OnItemAcquired);
	InventoryManagerComponent->OnItemRemovedDelegate.AddDynamic(this, &UGunnerInventoryViewModel_Equipment::OnItemRemoved);

	UNexusProperty* SlotIndexProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_SlotIndex);
	check(SlotIndexProperty);
	SlotIndexProperty->OnDirtyDelegate.AddDynamic(this, &UGunnerInventoryViewModel_Equipment::OnSlotIndexDirty);
	OnSlotIndexDirty(static_cast<float>(EquippedSlotType), SlotIndexProperty->GetDynamicValue());
}

void UGunnerInventoryViewModel_Equipment::OnDestroyViewModel(const UObject* Object, const UMVVMView* View) const
{
	Super::OnDestroyViewModel(Object, View);
	if (InventoryManagerComponent)
	{
		InventoryManagerComponent->OnItemAcquiredDelegate.RemoveDynamic(this, &UGunnerInventoryViewModel_Equipment::OnItemAcquired);
		InventoryManagerComponent->OnItemRemovedDelegate.RemoveDynamic(this, &UGunnerInventoryViewModel_Equipment::OnItemRemoved);
	}

	if (ActionComponent)
	{
		if (UNexusProperty* SlotIndexProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_SlotIndex))
		{
			SlotIndexProperty->OnDirtyDelegate.RemoveDynamic(this, &UGunnerInventoryViewModel_Equipment::OnSlotIndexDirty);
		}
	}
}

void UGunnerInventoryViewModel_Equipment::OnItemAcquired(AGunnerItem* Item)
{
	AGunnerSlotItem* SlotItem = Cast<AGunnerSlotItem>(Item);
	if (!SlotItem)
	{
		return;
	}

	const UGunnerItemDef* ItemDef = SlotItem->GetItemDef();
	if (!ItemDef)
	{
		return;
	}

	if (const UGunnerItemPart_Icon* IconPart = ItemDef->FindItemPartByClass<UGunnerItemPart_Icon>())
	{
		FGunnerEquipmentHudDataEntry& Entry = EquipmentHudData.FindOrAdd(SlotItem->GetSlotType());
		Entry.ItemNames = IconPart->DisplayNameText;
		Entry.StandardIconBrush = IconPart->StandardIconBrush;
		Entry.SimplifiedIconBrush = IconPart->SimplifiedIconBrush;
	}
	else
	{
		EquipmentHudData.Remove(SlotItem->GetSlotType());
	}

	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(EquipmentHudData);
}


void UGunnerInventoryViewModel_Equipment::OnItemRemoved(AGunnerItem* Item)
{
	AGunnerSlotItem* SlotItem = Cast<AGunnerSlotItem>(Item);
	if (!SlotItem)
	{
		return;
	}

	EquipmentHudData.Remove(SlotItem->GetSlotType());
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(EquipmentHudData);
}

void UGunnerInventoryViewModel_Equipment::OnSlotIndexDirty(float OldValue, float NewValue)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(EquippedSlotType, static_cast<EGunnerSlotType>(NewValue)))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(EquippedSlotType);
	}
}
