// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionDropSlotItem.h"

#include "Action/NexusActionComponent.h"
#include "Gunner/Slot/GunnerInventoryManagerComponent.h"
#include "Gunner/Slot/GunnerSlotItem.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"
#include "Gunner/_Core/GunnerSlotItemPickup.h"

UGunnerActionDropSlotItem::UGunnerActionDropSlotItem()
{
	ActionNetMethod = ENexusActionNetMethod::ServerOnly;
	bAllowRemoteTrigger = true;
	ActionTriggerEventTags.AddTag(GunnerNativeGameplayTags::TAG_Input_Drop);
}

void UGunnerActionDropSlotItem::OnActionAdded()
{
	Super::OnActionAdded();


	if (!PickupClass)
	{
		PickupClass = AGunnerSlotItemPickup::StaticClass();
	}

	InventoryManagerComponent = UGunnerInventoryManagerComponent::GetInventoryManagerComponentFromActor(GetAgentActor());
	ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetAgentActor());
}

bool UGunnerActionDropSlotItem::OnCanTriggerAction() const
{
	UNexusProperty* SlotIndexProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_SlotIndex);
	check(SlotIndexProperty);
	EGunnerSlotType CurrentSlotType = static_cast<EGunnerSlotType>(SlotIndexProperty->GetDynamicValue());
	return CurrentSlotType == EGunnerSlotType::Primary ||
		CurrentSlotType == EGunnerSlotType::Secondary ||
		CurrentSlotType == EGunnerSlotType::Spike;
}

void UGunnerActionDropSlotItem::OnTriggerAction()
{
	Super::OnTriggerAction();

	AGunnerSlotItem* ItemToDrop = nullptr;
	for (AGunnerItem* Item : InventoryManagerComponent->GetInventoryItems())
	{
		AGunnerSlotItem* SlotItem = Cast<AGunnerSlotItem>(Item);
		if (SlotItem && SlotItem->GetSlotType() == GetCurrentSlotType())
		{
			// Drop the item
			ItemToDrop = SlotItem;
			InventoryManagerComponent->AuthRemoveItem(ItemToDrop, false);
			break;
		}
	}


	ActionComponent->SendEventToSelf<FNexusEventMessage>(GunnerNativeGameplayTags::TAG_GameEvent_CycleSlot, {});


	AGunnerSlotItemPickup* PickupActor = GetWorld()->SpawnActorDeferred<AGunnerSlotItemPickup>(PickupClass, FTransform::Identity);
	check(PickupActor)
	PickupActor->InitializeSlotItemPickup(ItemToDrop);

	FTransform SpawnTransform = GetAgentActor()->GetActorTransform();
	FVector EyeLocation;
	FRotator EyeRotation;
	GetAgentActor()->GetActorEyesViewPoint(EyeLocation, EyeRotation);
	PickupActor->FinishSpawning(FTransform{EyeRotation, EyeLocation});
	EndAction();
}

EGunnerSlotType UGunnerActionDropSlotItem::GetCurrentSlotType() const
{
	UNexusProperty* SlotIndexProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_SlotIndex);
	check(SlotIndexProperty);
	return static_cast<EGunnerSlotType>(SlotIndexProperty->GetDynamicValue());
}

UGunnerActionCycleSlotItem::UGunnerActionCycleSlotItem()
{
	ActionNetMethod = ENexusActionNetMethod::ServerOnly;
	bAllowRemoteTrigger = true;
	ActionTriggerEventTags.AddTag(GunnerNativeGameplayTags::TAG_GameEvent_CycleSlot);
}

void UGunnerActionCycleSlotItem::OnActionAdded()
{
	Super::OnActionAdded();
	InventoryManagerComponent = UGunnerInventoryManagerComponent::GetInventoryManagerComponentFromActor(GetAgentActor());
}

bool UGunnerActionCycleSlotItem::OnCanTriggerAction() const
{
	bool bCanTrigger = Super::OnCanTriggerAction();
	if (!bCanTrigger)
	{
		return false;
	}
	return InventoryManagerComponent != nullptr;
}

void UGunnerActionCycleSlotItem::OnTriggerAction()
{
	Super::OnTriggerAction();
	FGameplayTag EventTag;


	const TArray<AGunnerItem*> Items = InventoryManagerComponent->GetInventoryItems();
	AGunnerItem* const* FoundItemPtr = Items.FindByPredicate([](const AGunnerItem* Item)
		{
			const AGunnerSlotItem* SlotItem = Cast<AGunnerSlotItem>(Item);
			return SlotItem && (SlotItem->GetSlotType() == EGunnerSlotType::Primary || SlotItem->GetSlotType() == EGunnerSlotType::Secondary ||
				SlotItem->GetSlotType() == EGunnerSlotType::Melee);
		}
	);

	checkf(FoundItemPtr && *FoundItemPtr, TEXT("장착할 아이템이 없습니다."));
	

	AGunnerSlotItem* ActivableItem = Cast<AGunnerSlotItem>(*FoundItemPtr);

	switch (ActivableItem->GetSlotType())
	{
	case EGunnerSlotType::Primary:
		EventTag = GunnerNativeGameplayTags::TAG_Input_ActivateSlot_Primary;
		break;
	case EGunnerSlotType::Secondary:
		EventTag = GunnerNativeGameplayTags::TAG_Input_ActivateSlot_Secondary;
		break;
	case EGunnerSlotType::Melee:
		EventTag = GunnerNativeGameplayTags::TAG_Input_ActivateSlot_Melee;
		break;
	default:
		checkNoEntry();
		break;
	}

	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetAgentActor());
	check(ActionComponent);
	ActionComponent->SendEventToSelf<FNexusEventMessage>(EventTag, {});
	EndAction();
}
