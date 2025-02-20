// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerDamageType.h"


float UGunnerDamageType::CalculateDamageByContext(const FDamageContext& DamageContext) const
{
	check(DamageContext.IsValid());
	return DamageContext.bIsAlt ? BaseDamage * AltDamageMultiplier : BaseDamage;
}