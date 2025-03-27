// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerEquipmentDef.h"

#include "Gunner/_Core/Animation/GunnerInstancedAnimSet.h"
#include "Gunner/_Core/Damage/GunnerDamageType.h"
#include "Misc/DataValidation.h"


#if WITH_EDITOR
EDataValidationResult UGunnerEquipmentDef::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	// 총기 교체를 통하여 장전 속도를 빠르게 할 수 없도록 설정
	if (ReloadSpeed > 0.0f && (ReloadSpeed / 2.0f >= EquipSpeed))
	{
		Result = CombineDataValidationResults(Result, EDataValidationResult::NotValidated);
		Context.AddError(FText::Format(NSLOCTEXT("Gunner", "InvalidEquipSpeed", "장비 속도가 장전 속도의 절반 이상입니다. (장비 속도: {0}, 장전 속도: {1})"), EquipSpeed, ReloadSpeed));
		return Result;
	}

	if (DamageType)
	{
		Result = CombineDataValidationResults(Result, DamageType->IsDataValid(Context));
	}
	return Result;
}
#endif


float UGunnerEquipmentDef::CalculateDamageByContext(UGunnerDamageContext* DamageContext) const
{
	return DamageType ? DamageType->CalculateDamageByContext(DamageContext) : 0.0f;
}

UGunnerInstancedAnimSet* UGunnerEquipmentDef::FindInstancedAnimSetByClass(TSubclassOf<UGunnerInstancedAnimSet> AnimSetClass) const
{
	for (UGunnerInstancedAnimSet* InstancedAnimSet : InstancedAnimSets)
	{
		if (InstancedAnimSet->IsA(AnimSetClass))
		{
			return InstancedAnimSet;
		}
	}
	return nullptr;
}
