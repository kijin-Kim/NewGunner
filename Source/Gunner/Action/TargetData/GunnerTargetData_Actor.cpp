// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerTargetData_Actor.h"

bool FGunnerTargetData_Actor::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	Ar << Actor;
	return bOutSuccess;
}
