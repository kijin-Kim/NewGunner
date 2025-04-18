// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerTestGun.h"

#include "Action/NexusActionComponent.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"


UGunnerTestActionActivatePrimary::UGunnerTestActionActivatePrimary()
{
	ActionNetMethod = ENexusActionNetMethod::ServerOnly;
	ActionTriggerEventTags.AddTag(GunnerNativeGameplayTags::TAG_Input_ActivateSlot_Primary);
}

// Sets default values
AGunnerTestGun::AGunnerTestGun()
{
	PrimaryActorTick.bCanEverTick = true;
	SlotType = EGunnerSlotType::Primary;

	PersistentActivationActions.Add(UGunnerTestActionActivatePrimary::StaticClass());
	TransientActivationActions.Add(UGunnerTestActionTransient::StaticClass());
}

