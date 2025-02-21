// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerEquipmentDef.h"

#include "Gunner/_Core/Animation/GunnerInstancedAnimSet.h"
#include "Gunner/_Core/Damage/GunnerDamageType.h"


#if WITH_EDITOR
EDataValidationResult UGunnerEquipmentDef::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	if (DamageType)
	{
		Result = CombineDataValidationResults(Result, DamageType->IsDataValid(Context));
	}
	return Result;
}
#endif


float UGunnerEquipmentDef::CalculateDamageByContext(const FDamageContext& DamageContext) const
{
	return DamageType ? DamageType->CalculateDamageByContext(DamageContext) : 0.0f;
}
