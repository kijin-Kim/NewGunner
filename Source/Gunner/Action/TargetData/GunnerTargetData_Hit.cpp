// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerTargetData_Hit.h"

bool FGunnerTargetData_Hit::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	Ar << TimeStamp;
	Ar << AgentActor;
	uint8 HitResultCount = HitResults.Num();
	Ar << HitResultCount;

	if (Ar.IsLoading())
	{
		HitResults.SetNum(HitResultCount);
	}

	for (FHitResult& HitResult : HitResults)
	{
		HitResult.NetSerialize(Ar, Map, bOutSuccess);
	}

	return !bOutSuccess || Ar.IsError();
}
