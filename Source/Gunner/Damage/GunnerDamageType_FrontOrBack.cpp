// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerDamageType_FrontOrBack.h"

#include "Gunner/_Core/Damage/GunnerDamageContext.h"

float UGunnerDamageType_FrontOrBack::CalculateDamageByContext(const FGunnerDamageContext& DamageContext) const
{
	const float Damage = Super::CalculateDamageByContext(DamageContext);
	return DamageContext.HitNormal.Z > 0.0f ? Damage : Damage * BackDamageMultiplier;
}
