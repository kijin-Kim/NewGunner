// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionEquipmentBase.h"

#include "Gunner/Equipment/GunnerEquipmentDef.h"
#include "Gunner/Equipment/GunnerEquipmentManagerComponent.h"

void UGunnerActionEquipmentBase::OnActionAdded()
{
	Super::OnActionAdded();
	EquipmentManager = GetAgentActor()->GetComponentByClass<UGunnerEquipmentManagerComponent>();
	check(EquipmentManager);
}

UGunnerEquipmentDef* UGunnerActionEquipmentBase::GetEquipmentDef() const
{
	AGunnerEquipment* Equipment = GetEquipment();
	return Equipment ? Equipment->GetEquipmentDef() : nullptr;
}

AGunnerEquipment* UGunnerActionEquipmentBase::GetEquipment() const
{
	return Cast<AGunnerEquipment>(GetSourceObject());
}
