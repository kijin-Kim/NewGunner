// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerInventoryViewModel_SlotItem.h"

#include "Action/NexusActionComponent.h"
#include "Gunner/Item/GunnerInventoryManagerComponent.h"
#include "Gunner/Item/GunnerItemDef.h"
#include "Gunner/Item/GunnerSlotItem.h"
#include "Gunner/Item/Part/GunnerItemPart_AmountView.h"
#include "Gunner/Item/Part/GunnerItemPart_Icon.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"


void UGunnerInventoryViewModel_SlotItem::OnCreateViewModel(const UUserWidget* UserWidget)
{
	Super::OnCreateViewModel(UserWidget);

	if (!InventoryManagerComponent)
	{
		return;
	}

	const TArray<AGunnerItem*>& Items = InventoryManagerComponent->GetInventoryItems();

	for (AGunnerItem* Item : Items)
	{
		if (AGunnerSlotItem* SlotItem = Cast<AGunnerSlotItem>(Item))
		{
			OnItemAcquired(SlotItem);
		}
	}

	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(SlotItemHudData);

	InventoryManagerComponent->OnItemAcquiredDelegate.AddDynamic(this, &UGunnerInventoryViewModel_SlotItem::OnItemAcquired);
	InventoryManagerComponent->OnItemRemovedDelegate.AddDynamic(this, &UGunnerInventoryViewModel_SlotItem::OnItemRemoved);

	UNexusProperty* SlotIndexProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_SlotIndex);
	check(SlotIndexProperty);
	SlotIndexProperty->OnDirtyDelegate.AddDynamic(this, &UGunnerInventoryViewModel_SlotItem::OnSlotIndexDirty);
	OnSlotIndexDirty(static_cast<float>(ActiveSlotType), SlotIndexProperty->GetDynamicValue());
}

void UGunnerInventoryViewModel_SlotItem::OnDestroyViewModel(const UObject* Object, const UMVVMView* View) const
{
	Super::OnDestroyViewModel(Object, View);
	if (InventoryManagerComponent)
	{
		InventoryManagerComponent->OnItemAcquiredDelegate.RemoveDynamic(this, &UGunnerInventoryViewModel_SlotItem::OnItemAcquired);
		InventoryManagerComponent->OnItemRemovedDelegate.RemoveDynamic(this, &UGunnerInventoryViewModel_SlotItem::OnItemRemoved);
	}

	if (ActionComponent)
	{
		if (UNexusProperty* SlotIndexProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_SlotIndex))
		{
			SlotIndexProperty->OnDirtyDelegate.RemoveDynamic(this, &UGunnerInventoryViewModel_SlotItem::OnSlotIndexDirty);
		}
	}
}

void UGunnerInventoryViewModel_SlotItem::OnItemAcquired(AGunnerItem* Item)
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
		FGunnerSlotItemHudDataEntry& Entry = SlotItemHudData.FindOrAdd(SlotItem->GetSlotType());
		Entry.ItemNames = IconPart->DisplayNameText;
		Entry.StandardIconBrush = IconPart->StandardIconBrush;
		Entry.SimplifiedIconBrush = IconPart->SimplifiedIconBrush;
	}
	else
	{
		SlotItemHudData.Remove(SlotItem->GetSlotType());
	}


	if (const UGunnerItemPart_AmountView* AmountPart = ItemDef->FindItemPartByClass<UGunnerItemPart_AmountView>())
	{
		TSubclassOf<UUserWidget>& ViewClass = CachedSlotItemWidgetClasses.FindOrAdd(SlotItem->GetSlotType());
		ViewClass = AmountPart->ViewClass;
	}
	else
	{
		CachedSlotItemWidgetClasses.Remove(SlotItem->GetSlotType());
	}

	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(SlotItemHudData);
}


void UGunnerInventoryViewModel_SlotItem::OnItemRemoved(AGunnerItem* Item)
{
	AGunnerSlotItem* SlotItem = Cast<AGunnerSlotItem>(Item);
	if (!SlotItem)
	{
		return;
	}

	SlotItemHudData.Remove(SlotItem->GetSlotType());
	CachedSlotItemWidgetClasses.Remove(SlotItem->GetSlotType());
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(SlotItemHudData);
}

void UGunnerInventoryViewModel_SlotItem::OnSlotIndexDirty(float OldValue, float NewValue)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(ActiveSlotType, static_cast<EGunnerSlotType>(NewValue)))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(ActiveSlotType);
	}
	
	if (TSubclassOf<UUserWidget>* NewWidgetClassPtr = CachedSlotItemWidgetClasses.Find(static_cast<EGunnerSlotType>(NewValue)))
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(CurrentAmountWidgetClass, *NewWidgetClassPtr))
		{
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CurrentAmountWidgetClass);
		}
	}
	else
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(CurrentAmountWidgetClass, nullptr))
		{
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CurrentAmountWidgetClass);
		}
	}
}
