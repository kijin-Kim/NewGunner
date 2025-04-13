// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionSlotItemBase.h"

#include "Action/NexusActionComponent.h"
#include "Gunner/Slot/GunnerGun.h"

AGunnerSlotItem* UGunnerActionSlotItemBase::GetSlotItem() const
{
	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetAgentActor());
	check(ActionComponent);
	FNexusActionDef* ActionDef = ActionComponent->FindActionDefByHandle(GetActionDefHandle());
	check(ActionDef && ActionDef->ActionInstance && ActionDef->ActionInstance == this);
	return Cast<AGunnerSlotItem>(ActionDef->SourceObject);
}
