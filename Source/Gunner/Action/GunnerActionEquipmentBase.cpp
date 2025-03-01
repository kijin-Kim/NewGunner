// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionEquipmentBase.h"

#include "Gunner/Gunner.h"
#include "Gunner/Equipment/GunnerEquipmentDef.h"
#include "Gunner/Equipment/GunnerEquipmentManagerComponent.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"

void UGunnerActionEquipmentBase::OnActionAdded_Implementation()
{
	Super::OnActionAdded_Implementation();
	EquipmentManager = GetAgentActor()->GetComponentByClass<UGunnerEquipmentManagerComponent>();
	check(EquipmentManager);

	AGunnerEquipment* Equipment = GetEquipment();
	switch (Equipment->GetEquipmentType())
	{
	case EEquipmentType::Primary:
		ActionTriggerEventTags.AddTag(TAG_Input_Equip_Primary);
		break;
	case EEquipmentType::Secondary:
		ActionTriggerEventTags.AddTag(TAG_Input_Equip_Secondary);
		break;
	case EEquipmentType::Melee:
		ActionTriggerEventTags.AddTag(TAG_Input_Equip_Melee);
		break;
	case EEquipmentType::CSkill:
	case EEquipmentType::QSkill:
	case EEquipmentType::ESkill:
	case EEquipmentType::XSkill:
	case EEquipmentType::EquipmentTypeCount:
		unimplemented();
		break;
	}
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
