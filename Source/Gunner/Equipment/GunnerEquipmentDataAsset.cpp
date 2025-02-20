// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerEquipmentDataAsset.h"

#include "Gunner/_Core/Animation/GunnerInstancedAnimSet.h"
#include "Gunner/_Core/Damage/GunnerDamageType.h"


#if WITH_EDITOR
EDataValidationResult UGunnerEquipmentDataAsset::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	if (DamageType)
	{
		Result = CombineDataValidationResults(Result, DamageType->IsDataValid(Context));
	}
	return Result;
}
#endif

UGunnerInstancedAnimSet* UGunnerEquipmentDataAsset::GetAnimSetByClass(TSubclassOf<UGunnerInstancedAnimSet> AnimSetClass) const
{
	for (TObjectPtr<UGunnerInstancedAnimSet> AnimSet : InstancedAnimSets)
	{
		if (AnimSet && AnimSet->GetClass() == AnimSetClass)
		{
			return AnimSet.Get();
		}
	}
	return nullptr;
}

float UGunnerEquipmentDataAsset::CalculateDamageByContext(const FDamageContext& DamageContext) const
{
	return DamageType ? DamageType->CalculateDamageByContext(DamageContext) : 0.0f;
}
