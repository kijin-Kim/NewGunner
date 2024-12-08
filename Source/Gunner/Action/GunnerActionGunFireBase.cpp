// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionGunFireBase.h"

#include "Gunner/Equipment/GunnerEquipment.h"
#include "Gunner/Equipment/GunnerEquipmentManagerComponent.h"

void UGunnerActionGunFireBase::FireHitScan(TArray<FHitResult>& OutHitResults)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	AActor* AgentActor = GetAgentActor();
	check(AgentActor);
	UGunnerEquipmentManagerComponent* EquipmentManagerComponent = AgentActor->GetComponentByClass<UGunnerEquipmentManagerComponent>();
	check(EquipmentManagerComponent);

	OutHitResults.Empty();
	EquipmentManagerComponent->LocalHitScan(OutHitResults);
	if (OutHitResults.IsEmpty())
	{
		return;
	}

	if (IsOwnerActorAuthoritative())
	{
		EquipmentManagerComponent->AuthApplyDamage(OutHitResults);
		return;
	}

	FVector Location;
	FRotator Rotation;
	AgentActor->GetActorEyesViewPoint(Location, Rotation);
	TArray<FClientHitScanData> ClientHitScanData;
	for (const FHitResult& HitResult : OutHitResults)
	{
		ClientHitScanData.Add(FClientHitScanData(HitResult, Location, Rotation));
	}
	EquipmentManagerComponent->ServerRequestHitScanConfirm(ClientHitScanData);
}

UHitScanSignDataObject* UGunnerActionGunFireBase::MakeHitScanSignDataObject(AActor* AgentActor, const TArray<FHitResult>& HitResults)
{
	UHitScanSignDataObject* HitScanSignDataObject = NewObject<UHitScanSignDataObject>(AgentActor);
	HitScanSignDataObject->AgentActor = AgentActor;
	HitScanSignDataObject->LocalHitResult = HitResults;
	return HitScanSignDataObject;
}
