// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerEquipmentManagerComponent.h"

#include "GunnerEquipment.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UGunnerEquipmentManagerComponent::UGunnerEquipmentManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	EquipmentSlots.SetNum(MaxSlots);
	SetIsReplicatedByDefault(true);
}

void UGunnerEquipmentManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGunnerEquipmentManagerComponent, EquipmentSlots);
	DOREPLIFETIME(UGunnerEquipmentManagerComponent, CurrentEquipment);
}

void UGunnerEquipmentManagerComponent::AddEquipmentToSlot(int32 SlotIndex, TSubclassOf<AGunnerEquipment> EquipmentClass)
{
	if (!EquipmentSlots.IsValidIndex(SlotIndex))
	{
		return;
	}

	AActor* ActorOwner = GetOwner();
	check(ActorOwner);

	if (EquipmentSlots[SlotIndex])
	{
		EquipmentSlots[SlotIndex]->RemoveActionsOnAcquire(ActorOwner);
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = ActorOwner;
	AGunnerEquipment* Equipment = GetWorld()->SpawnActor<AGunnerEquipment>(EquipmentClass, SpawnParams);
	Equipment->AddActionsOnAcquire();
	EquipmentSlots[SlotIndex] = Equipment;
}

void UGunnerEquipmentManagerComponent::SetCurrentEquipmentByIndex(int32 SlotIndex)
{
	if (EquipmentSlots.IsValidIndex(SlotIndex) && EquipmentSlots[SlotIndex])
	{
		AGunnerEquipment* LastEquipment = CurrentEquipment;
		CurrentEquipment = EquipmentSlots[SlotIndex];
		OnRep_CurrentEquipment(LastEquipment);
	}
}

AGunnerEquipment* UGunnerEquipmentManagerComponent::GetEquipmentByIndex(int32 SlotIndex) const
{
	if (EquipmentSlots.IsValidIndex(SlotIndex))
	{
		return EquipmentSlots[SlotIndex];
	}
	return nullptr;
}

AGunnerEquipment* UGunnerEquipmentManagerComponent::GetCurrentEquipment() const
{
	return CurrentEquipment;
}

void UGunnerEquipmentManagerComponent::OnRep_CurrentEquipment(AGunnerEquipment* LastEquipment)
{
	if (LastEquipment)
	{
		LastEquipment->OnUnequipped();
	}

	if (CurrentEquipment)
	{
		CurrentEquipment->OnEquipped();
	}
}
