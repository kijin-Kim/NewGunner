// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerDamageType_FrontOrBack.h"

float UGunnerDamageType_FrontOrBack::CalculateDamageByContext(const FDamageContext& DamageContext) const
{
	check(DamageContext.IsValid());
	const float Damage = Super::CalculateDamageByContext(DamageContext);
	return DamageContext.HitNormal.Z > 0.0f ? Damage : Damage * BackDamageMultiplier;
}
