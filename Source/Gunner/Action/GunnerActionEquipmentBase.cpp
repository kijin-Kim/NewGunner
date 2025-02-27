// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionEquipmentBase.h"

#include "Gunner/Equipment/GunnerEquipmentDef.h"
#include "Gunner/Equipment/GunnerEquipmentManagerComponent.h"

void UGunnerActionEquipmentBase::OnActionAdded_Implementation()
{
	Super::OnActionAdded_Implementation();
	EquipmentManager = GetAgentActor()->GetComponentByClass<UGunnerEquipmentManagerComponent>();
	check(EquipmentManager);
	AGunnerEquipment* SourceEquipment = Cast<AGunnerEquipment>(GetSourceObject());
	check(SourceEquipment);
	EquipmentDef = SourceEquipment->GetEquipmentDef();
	check(EquipmentDef);
}
