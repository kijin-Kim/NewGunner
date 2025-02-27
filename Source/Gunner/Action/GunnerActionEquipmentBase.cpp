// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionEquipmentBase.h"
#include "Gunner/Equipment/GunnerEquipmentManagerComponent.h"

void UGunnerActionEquipmentBase::OnActionAdded_Implementation()
{
	Super::OnActionAdded_Implementation();
	EquipmentManager = GetAgentActor()->GetComponentByClass<UGunnerEquipmentManagerComponent>();
	check(EquipmentManager);
	Equipment = Cast<AGunnerEquipment>(GetSourceObject());
	check(Equipment);
	EquipmentDef = Equipment->GetEquipmentDef();
	check(EquipmentDef);
}
