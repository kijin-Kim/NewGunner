// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerSlotItemPickup.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Gunner/Equipment/GunnerEquipment.h"
#include "Gunner/Slot/GunnerInventoryManagerComponent.h"
#include "Gunner/Slot/GunnerInventoryManagerInterface.h"
#include "Gunner/Slot/GunnerSlotItem.h"
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
		SlotItemInstance = InSlotItemInstance;
		CopyMeshFromSource();
	}
}

void AGunnerSlotItemPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(AGunnerSlotItemPickup, SlotItemInstance, COND_InitialOnly, REPNOTIFY_Always);
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

	if (!SlotItemInstance)
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
	return InventoryManager->CanAcquireItem(SlotItemInstance);
}

void AGunnerSlotItemPickup::OnPickup_Implementation(AActor* OtherActor, UActorComponent* OtherComponent)
{
	Super::OnPickup_Implementation(OtherActor, OtherComponent);

	IGunnerInventoryManagerInterface* SlotManagerInterface = Cast<IGunnerInventoryManagerInterface>(OtherActor);
	check(SlotManagerInterface);
	UGunnerInventoryManagerComponent* SlotManager = SlotManagerInterface->GetInventoryManagerComponent();
	check(SlotManager);
	SlotManager->AuthAddItem(SlotItemInstance);
}

void AGunnerSlotItemPickup::CopyMeshFromSource()
{
	if (!PickupMeshComponent || !SlotItemInstance || !SlotItemInstance->Implements<UNexusAnimMontagePlayerInterface>())
	{
		return;
	}

	USkeletalMeshComponent* SourceEquipmentMeshComponent = INexusAnimMontagePlayerInterface::Execute_GetFirstPersonMeshComponent(SlotItemInstance);
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

void AGunnerSlotItemPickup::OnRep_EquipmentInstance()
{
	CopyMeshFromSource();
}
