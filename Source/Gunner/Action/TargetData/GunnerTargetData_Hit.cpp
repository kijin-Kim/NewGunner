// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerTargetData_Hit.h"

bool FGunnerTargetData_Hit::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	Ar << AgentActor;
	uint8 HitResultCount = LocalHitResult.Num();
	Ar << HitResultCount;

	if (Ar.IsLoading())
	{
		LocalHitResult.SetNum(HitResultCount);
	}

	for (FHitResult& HitResult : LocalHitResult)
	{
		HitResult.NetSerialize(Ar, Map, bOutSuccess);
	}

	return !bOutSuccess || Ar.IsError();
}
