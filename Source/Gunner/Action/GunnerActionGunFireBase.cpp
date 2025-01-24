// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionGunFireBase.h"

#include "Net/UnrealNetwork.h"


void UHitScanSignDataObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHitScanSignDataObject, AgentActor);
	DOREPLIFETIME(UHitScanSignDataObject, LocalHitResult);
}

UHitScanSignDataObject* UGunnerActionGunFireBase::MakeHitScanSignDataObject(AActor* AgentActor, const TArray<FHitResult>& HitResults)
{
	UHitScanSignDataObject* HitScanSignDataObject = NewObject<UHitScanSignDataObject>(AgentActor);
	HitScanSignDataObject->AgentActor = AgentActor;
	HitScanSignDataObject->LocalHitResult = HitResults;
	return HitScanSignDataObject;
}
