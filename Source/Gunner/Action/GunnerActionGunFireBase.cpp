// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionGunFireBase.h"

#include "Gunner/Equipment/GunnerEquipment.h"
#include "Gunner/Equipment/GunnerEquipmentManagerComponent.h"

void UGunnerActionGunFireBase::FireHitScan()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	AActor* AgentActor = GetAgentActor();
	check(AgentActor);
	UGunnerEquipmentManagerComponent* EquipmentManagerComponent = AgentActor->GetComponentByClass<UGunnerEquipmentManagerComponent>();
	check(EquipmentManagerComponent);

	TArray<FHitResult> HitResults;
	EquipmentManagerComponent->LocalHitScan(HitResults);
	if (HitResults.IsEmpty())
	{
		return;
	}

	if (IsOwnerActorAuthoritative())
	{
		EquipmentManagerComponent->AuthApplyDamage(HitResults);
		return;
	}

	FVector Location;
	FRotator Rotation;
	AgentActor->GetActorEyesViewPoint(Location, Rotation);
	TArray<FClientHitScanData> ClientHitScanData;
	for (const FHitResult& HitResult : HitResults)
	{
		ClientHitScanData.Add(FClientHitScanData(HitResult, Location, Rotation));
	}
	EquipmentManagerComponent->ServerRequestHitScanConfirm(ClientHitScanData);
}
