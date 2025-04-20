// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerItemHudViewModel.h"

#include "Gunner/Slot/GunnerInventoryManagerComponent.h"
#include "Gunner/Slot/GunnerSlotItem.h"
#include "Gunner/Slot/UI/Data/GunnerSlotItemUiData.h"


void UGunnerItemHudViewModel::OnCreateViewModel(const UUserWidget* UserWidget)
{
	Super::OnCreateViewModel(UserWidget);

	ItemHudData.SetNum(static_cast<int>(EGunnerSlotType::Num));


	const TArray<AGunnerItem*>& Items = InventoryManagerComponent->GetInventoryItems();

	for (int i = 0; i < static_cast<int>(EGunnerSlotType::Num); i++)
	{
		if (AGunnerSlotItem* SlotItem = Cast<AGunnerSlotItem>(Items[i]))
		{
			OnItemAcquired(SlotItem);
		}
		else
		{
			ItemHudData[i].Reset();
		}
	}

	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(ItemHudData);

	InventoryManagerComponent->OnItemAcquiredDelegate.AddDynamic(this, &UGunnerItemHudViewModel::OnItemAcquired);
	InventoryManagerComponent->OnItemRemovedDelegate.AddDynamic(this, &UGunnerItemHudViewModel::OnItemRemoved);
}

void UGunnerItemHudViewModel::OnDestroyViewModel(const UObject* Object, const UMVVMView* View) const
{
	Super::OnDestroyViewModel(Object, View);
	if (InventoryManagerComponent)
	{
		InventoryManagerComponent->OnItemAcquiredDelegate.RemoveDynamic(this, &UGunnerItemHudViewModel::OnItemAcquired);
		InventoryManagerComponent->OnItemRemovedDelegate.RemoveDynamic(this, &UGunnerItemHudViewModel::OnItemRemoved);
	}
}

void UGunnerItemHudViewModel::OnItemAcquired(AGunnerItem* Item)
{
	AGunnerSlotItem* SlotItem = Cast<AGunnerSlotItem>(Item);
	if (!SlotItem)
	{
		return;
	}


	if (const UGunnerSlotItemUiData* UiData = SlotItem->GetSlotItemUIData())
	{
		ItemHudData[static_cast<int>(SlotItem->GetSlotType())] = FGunnerItemHudDataEntry{
			UiData->ItemName,
			UiData->StandardIconTexture,
			UiData->SimplifiedIconTexture
		};
	}
	else
	{
		ItemHudData[static_cast<int>(SlotItem->GetSlotType())].Reset();
	}

	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(ItemHudData);
}


void UGunnerItemHudViewModel::OnItemRemoved(AGunnerItem* Item)
{
	AGunnerSlotItem* SlotItem = Cast<AGunnerSlotItem>(Item);
	if (!SlotItem)
	{
		return;
	}


	ItemHudData[static_cast<int>(SlotItem->GetSlotType())] = FGunnerItemHudDataEntry{
		FText::GetEmpty(),
		nullptr,
		nullptr
	};
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(ItemHudData);
}
