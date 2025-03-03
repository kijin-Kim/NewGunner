// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerDamageType.h"

#include "Gunner/_Core/Damage/GunnerDamageContext.h"


float UGunnerDamageType::CalculateDamageByContext(UGunnerDamageContext* DamageContext) const
{
	check(DamageContext)
	return DamageContext->bIsAlt ? BaseDamage * AltDamageMultiplier : BaseDamage;
}
