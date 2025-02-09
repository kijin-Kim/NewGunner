// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerMapDataSet.h"
#include "GunnerMapData.h"

UGunnerMapData* UGunnerMapDataSet::GetMapDataByName(FString MapName) const
{
	auto MapDataPtr = TeamDeathMatchMaps.FindByPredicate([MapName](UGunnerMapData* MapData) { return MapData->MapName == MapName; });
	if (MapDataPtr)
	{
		return *MapDataPtr;
	}

	MapDataPtr = StandardAndDeathMatchMaps.FindByPredicate([MapName](UGunnerMapData* MapData) { return MapData->MapName == MapName; });
	if (MapDataPtr)
	{
		return *MapDataPtr;
	}

	return nullptr;
}
