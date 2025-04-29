// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerSlotItemPickup.h"

#include "Animation/NexusAnimMontagePlayerInterface.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Gunner/Item/GunnerInventoryManagerComponent.h"
#include "Gunner/Item/GunnerInventoryManagerInterface.h"
#include "Gunner/Item/GunnerSlotItem.h"
#include "Net/UnrealNetwork.h"


// Sets default values
AGunnerSlotItemPickup::AGunnerSlotItemPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	PickupMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PickupMeshComponent"));
	PickupMeshComponent->SetupAttachment(RootComponent);
	PickupMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
}

void AGunnerSlotItemPickup::InitializeSlotItemPickup(AGunnerSlotItem* InSlotItemInstance)
{
	if (InSlotItemInstance)
	{
		SlotItem = InSlotItemInstance;
		CopyMeshFromSource();
	}
}

void AGunnerSlotItemPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(AGunnerSlotItemPickup, SlotItem, COND_InitialOnly, REPNOTIFY_Always);
}

void AGunnerSlotItemPickup::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	CopyMeshFromSource();
}

bool AGunnerSlotItemPickup::CanPickup_Implementation(AActor* OtherActor, UActorComponent* OtherComponent) const
{
	bool bResult = Super::CanPickup_Implementation(OtherActor, OtherComponent);
	if (!bResult)
	{
		return false;
	}

	if (!OtherActor)
	{
		return false;
	}

	if (!SlotItem)
	{
		return false;
	}


	IGunnerInventoryManagerInterface* InventoryManagerInterface = Cast<IGunnerInventoryManagerInterface>(OtherActor);
	if (!InventoryManagerInterface)
	{
		return false;
	}

	UGunnerInventoryManagerComponent* InventoryManager = InventoryManagerInterface->GetInventoryManagerComponent();
	check(InventoryManager);
	return InventoryManager->CanAcquireItem(SlotItem);
}

void AGunnerSlotItemPickup::OnPickup_Implementation(AActor* OtherActor, UActorComponent* OtherComponent)
{
	Super::OnPickup_Implementation(OtherActor, OtherComponent);
	
	UGunnerInventoryManagerComponent* InventoryManager = UGunnerInventoryManagerComponent::GetInventoryManagerComponentFromActor(OtherActor);
	check(InventoryManager);
	InventoryManager->AuthAddItem(SlotItem);
}

void AGunnerSlotItemPickup::CopyMeshFromSource()
{
	if (!PickupMeshComponent || !SlotItem || !SlotItem->Implements<UNexusAnimMontagePlayerInterface>())
	{
		return;
	}

	USkeletalMeshComponent* SourceEquipmentMeshComponent = INexusAnimMontagePlayerInterface::Execute_GetFirstPersonMeshComponent(SlotItem);
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

void AGunnerSlotItemPickup::OnRep_SlotItemInstance()
{
	CopyMeshFromSource();
}
