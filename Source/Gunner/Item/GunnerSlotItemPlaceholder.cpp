// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerSlotItemPlaceholder.h"

#include "Action/NexusAction.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"


UGunnerActionSlotActivation_MeleePlaceholder::UGunnerActionSlotActivation_MeleePlaceholder()
{
	ActionNetMethod = ENexusActionNetMethod::ServerOnly;
	ActionTriggerEventTags.AddTag(GunnerNativeGameplayTags::TAG_Input_ActivateSlot_Melee);
	
}

void UGunnerActionSlotActivation_MeleePlaceholder::OnTriggerAction()
{
	Super::OnTriggerAction();
	EndAction();
}

AGunnerEquippable_MeleePlaceholder::AGunnerEquippable_MeleePlaceholder()
{
	SlotType = EGunnerSlotType::Melee;
	PersistentActivationActions.Add(UGunnerActionSlotActivation_MeleePlaceholder::StaticClass());
}
