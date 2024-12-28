// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionGunFireBase.h"


UHitScanSignDataObject* UGunnerActionGunFireBase::MakeHitScanSignDataObject(AActor* AgentActor, const TArray<FHitResult>& HitResults)
{
	UHitScanSignDataObject* HitScanSignDataObject = NewObject<UHitScanSignDataObject>(AgentActor);
	HitScanSignDataObject->AgentActor = AgentActor;
	HitScanSignDataObject->LocalHitResult = HitResults;
	return HitScanSignDataObject;
}
