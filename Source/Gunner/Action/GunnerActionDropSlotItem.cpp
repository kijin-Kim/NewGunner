// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionDropSlotItem.h"

#include "Action/NexusActionComponent.h"
#include "Gunner/Slot/GunnerSlotManagerComponent.h"
#include "Gunner/Slot/GunnerSlotManagerInterface.h"
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
	if (IGunnerSlotManagerInterface* SlotManagerInterface = Cast<IGunnerSlotManagerInterface>(GetAgentActor()))
	{
		SlotManagerComponent = SlotManagerInterface->GetSlotManagerComponent();
	}

	if (!PickupClass)
	{
		PickupClass = AGunnerSlotItemPickup::StaticClass();
	}
}

bool UGunnerActionDropSlotItem::OnCanTriggerAction() const
{
	return SlotManagerComponent && SlotManagerComponent->GetCurrentSlotItem()
		&& (SlotManagerComponent->GetCurrentSlotType() == EGunnerSlotType::Primary
			|| SlotManagerComponent->GetCurrentSlotType() == EGunnerSlotType::Secondary
			|| SlotManagerComponent->GetCurrentSlotType() == EGunnerSlotType::Spike);
}

void UGunnerActionDropSlotItem::OnTriggerAction()
{
	Super::OnTriggerAction();


	AGunnerSlotItem* ItemToDrop = SlotManagerComponent->GetCurrentSlotItem();
	check(ItemToDrop);
	SlotManagerComponent->AuthRemoveItemFromSlot(ItemToDrop, false);


	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetAgentActor());
	check(ActionComponent);
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

UGunnerActionCycleSlotItem::UGunnerActionCycleSlotItem()
{
	ActionNetMethod = ENexusActionNetMethod::ServerOnly;
	bAllowRemoteTrigger = true;
	ActionTriggerEventTags.AddTag(GunnerNativeGameplayTags::TAG_GameEvent_CycleSlot);
}

void UGunnerActionCycleSlotItem::OnActionAdded()
{
	Super::OnActionAdded();
	if (IGunnerSlotManagerInterface* SlotManagerInterface = Cast<IGunnerSlotManagerInterface>(GetAgentActor()))
	{
		SlotManagerComponent = SlotManagerInterface->GetSlotManagerComponent();
	}
}

bool UGunnerActionCycleSlotItem::OnCanTriggerAction() const
{
	bool bCanTrigger = Super::OnCanTriggerAction();
	if (!bCanTrigger)
	{
		return false;
	}
	return SlotManagerComponent != nullptr;
}

void UGunnerActionCycleSlotItem::OnTriggerAction()
{
	Super::OnTriggerAction();
	FGameplayTag EventTag;
	switch (SlotManagerComponent->FindActivableSlotType())
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
