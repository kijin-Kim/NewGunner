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

	FVector Location;
	FRotator Rotation;
	AgentActor->GetActorEyesViewPoint(Location, Rotation);

	UWorld* World = AgentActor->GetWorld();
	check(World);

	FCollisionQueryParams CollisionQueryParams;
	UGunnerEquipmentManagerComponent* EquipmentManagerComponent = AgentActor->GetComponentByClass<UGunnerEquipmentManagerComponent>();
	check(EquipmentManagerComponent);
	TArray<AActor*> IgnoredActors = {AgentActor, EquipmentManagerComponent->GetCurrentEquippedEquipment()};
	CollisionQueryParams.AddIgnoredActors(IgnoredActors);
	TArray<FHitResult> HitResult;
	World->LineTraceMultiByChannel(HitResult,
	                               Location,
	                               Location + Rotation.Vector() * 10000.0f,
	                               ECollisionChannel::ECC_Visibility, CollisionQueryParams, FCollisionResponseParams(ECR_Overlap));
	DrawDebugLine(World, Location, Location + Rotation.Vector() * 10000.0f, FColor::Red, false, 1.0f, 0, 1.0f);

	for (const FHitResult& Hit : HitResult)
	{
		DrawDebugSphere(World, Hit.ImpactPoint, 10.0f, 12, FColor::Red, false, 1.0f, 0, 1.0f);
	}

	if (!HitResult.IsEmpty())
	{
		TArray<FClientHitScanData> ClientHitScanData;
		for (const FHitResult& Hit : HitResult)
		{
			FClientHitScanData HitScanData;
			HitScanData.HitActor = Hit.GetActor();
			HitScanData.HitLocation = Hit.ImpactPoint;
			HitScanData.HitBoneName = Hit.BoneName;
			HitScanData.TimeStamp = World->GetTimeSeconds();
			HitScanData.ShooterLocation = Location;
			HitScanData.ShooterRotation = Rotation;
			ClientHitScanData.Add(HitScanData);
		}
		EquipmentManagerComponent->ServerFireHitScan(ClientHitScanData);
	}
}
