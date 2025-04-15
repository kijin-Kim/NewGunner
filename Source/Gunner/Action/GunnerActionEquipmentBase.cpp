// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionEquipmentBase.h"

#include "Action/NexusActionComponent.h"
#include "Gunner/Equipment/GunnerEquipmentDef.h"
#include "Gunner/Equipment/GunnerEquipmentManagerComponent.h"


UGunnerEquipmentDef* UGunnerActionEquipmentBase::GetEquipmentDef() const
{
	AGunnerEquipment* Equipment = GetEquipment();
	return Equipment ? Equipment->GetEquipmentDef() : nullptr;
}

AGunnerEquipment* UGunnerActionEquipmentBase::GetEquipment() const
{
	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetAgentActor());
	check(ActionComponent);
	FNexusActionDef* ActionDef = ActionComponent->FindActionDefByHandle(GetActionDefHandle());
	return Cast<AGunnerEquipment>(ActionDef->SourceObject);
}

void UGunnerActionEquipmentBase::OnActionAdded()
{
	Super::OnActionAdded();
	EquipmentManager = GetAgentActor()->GetComponentByClass<UGunnerEquipmentManagerComponent>();
	check(EquipmentManager);
}
