// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionEquipmentEquip.h"

#include "Gunner/Equipment/GunnerEquipment.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"

void UGunnerActionEquipmentEquip::OnActionAdded()
{
	Super::OnActionAdded();
	
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
