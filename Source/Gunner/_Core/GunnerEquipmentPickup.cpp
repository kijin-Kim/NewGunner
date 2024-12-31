// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerEquipmentPickup.h"

#include "Gunner/Equipment/GunnerEquipment.h"
#include "Gunner/Equipment/GunnerEquipmentManagerComponent.h"
#include "Net/UnrealNetwork.h"


// Sets default values
AGunnerEquipmentPickup::AGunnerEquipmentPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	PickupMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PickupMeshComponent"));
	PickupMeshComponent->SetupAttachment(RootComponent);
	PickupMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGunnerEquipmentPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(AGunnerEquipmentPickup, EquipmentInstance, COND_InitialOnly, REPNOTIFY_Always);
}

void AGunnerEquipmentPickup::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	CopyMeshFromSource();
}

bool AGunnerEquipmentPickup::CanPickup_Implementation(AActor* OtherActor, UActorComponent* OtherComponent) const
{
	bool bCanPickup = Super::CanPickup_Implementation(OtherActor, OtherComponent);
	if (!bCanPickup)
	{
		return false;
	}

	if (!OtherActor)
	{
		return false;
	}

	if (!EquipmentInstance)
	{
		return false;
	}

	UGunnerEquipmentManagerComponent* EquipmentManager = OtherActor->GetComponentByClass<UGunnerEquipmentManagerComponent>();
	if (!EquipmentManager)
	{
		return false;
	}


	return !IsValid(EquipmentManager->GetEquipmentByEquipmentType(EquipmentInstance->GetEquipmentType()));
}

void AGunnerEquipmentPickup::OnPickup_Implementation(AActor* OtherActor, UActorComponent* OtherComponent)
{
	Super::OnPickup_Implementation(OtherActor, OtherComponent);
	if (OtherActor)
	{
		if (UGunnerEquipmentManagerComponent* EquipmentManager = OtherActor->FindComponentByClass<UGunnerEquipmentManagerComponent>())
		{
			EquipmentManager->AuthAddEquipment(EquipmentInstance);
		}
	}
}

void AGunnerEquipmentPickup::CopyMeshFromSource()
{
	if (!PickupMeshComponent || !EquipmentInstance)
	{
		return;
	}


	USkeletalMeshComponent* SourceEquipmentMeshComponent = EquipmentInstance->GetFirstPersonMeshComponent();
	if (!SourceEquipmentMeshComponent)
	{
		return;
	}

	if (USkeletalMesh* MeshAsset = SourceEquipmentMeshComponent->GetSkeletalMeshAsset())
	{
		PickupMeshComponent->SetSkeletalMesh(MeshAsset);
	}

	TArray<UMaterialInterface*> Materials = SourceEquipmentMeshComponent->GetMaterials();
	for (int32 i = 0; i < Materials.Num(); ++i)
	{
		if (Materials[i])
		{
			PickupMeshComponent->SetMaterial(i, Materials[i]);
		}
	}
}

void AGunnerEquipmentPickup::OnRep_EquipmentInstance()
{
	CopyMeshFromSource();
}
