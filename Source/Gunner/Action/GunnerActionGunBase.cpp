// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionGunBase.h"

#include "NexusActionComponent.h"
#include "Gunner/Slot/GunnerGun.h"

AGunnerGun* UGunnerActionGunBase::GetGun() const
{
	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetAgentActor());
	check(ActionComponent);
	FNexusActionDef* ActionDef = ActionComponent->FindActionDefByHandle(GetActionDefHandle());
	check(ActionDef && ActionDef->ActionInstance && ActionDef->ActionInstance == this);
	return Cast<AGunnerGun>(ActionDef->SourceObject);
}
