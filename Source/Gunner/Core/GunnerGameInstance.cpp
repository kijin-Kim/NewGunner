// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerGameInstance.h"

UCurveTable* UGunnerGameInstance::GetDamageCurveTable() const
{
	return DamageCurveTable;
}

UDataTable* UGunnerGameInstance::GetWeaponDataTable() const
{
	return WeaponDataTable;
}
