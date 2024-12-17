// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionGunFireBase.h"

#include "GameFramework/Character.h"
#include "Gunner/Equipment/GunnerEquipmentManagerComponent.h"
#include "Gunner/Player/GunnerPlayerController.h"

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


	if (IsOwnerActorAuthoritative() && IsLocallyControlled())
	{
		TArray<AActor*> DamageActors;
		for (const FHitResult& HitResult : OutHitResults)
		{
			AActor* HitActor = HitResult.GetActor();
			if (HitActor && DamageActors.Find(HitActor) == INDEX_NONE)
			{
				DamageActors.Add(HitActor);
				EquipmentManagerComponent->AuthApplyDamage(HitActor, HitResult.BoneName, HitResult.Normal);
			}
		}
		return;
	}

	TArray<FClientHitScanData> ClientHitScanData;
	for (const FHitResult& HitResult : OutHitResults)
	{
		ClientHitScanData.Add(FClientHitScanData(HitResult));
	}

	AGunnerPlayerController* PlayerController = Cast<AGunnerPlayerController>(GetController());
	check(PlayerController);
	EquipmentManagerComponent->ServerRequestHitScanConfirm(ClientHitScanData, PlayerController->GetLocalServerTime());
}

UHitScanSignDataObject* UGunnerActionGunFireBase::MakeHitScanSignDataObject(AActor* AgentActor, const TArray<FHitResult>& HitResults)
{
	UHitScanSignDataObject* HitScanSignDataObject = NewObject<UHitScanSignDataObject>(AgentActor);
	HitScanSignDataObject->AgentActor = AgentActor;
	HitScanSignDataObject->LocalHitResult = HitResults;
	return HitScanSignDataObject;
}
