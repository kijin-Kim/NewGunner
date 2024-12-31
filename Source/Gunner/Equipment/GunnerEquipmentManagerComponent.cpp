// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerEquipmentManagerComponent.h"

#include "GunnerEquipment.h"
#include "Engine/Canvas.h"
#include "GameFramework/HUD.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Gunner/_Core/GunnerPickup.h"

#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UGunnerEquipmentManagerComponent::UGunnerEquipmentManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		AHUD::OnShowDebugInfo.AddStatic(&ThisClass::OnShowDebugInfo);
	}
}

void UGunnerEquipmentManagerComponent::InitEquipmentManagerComponent()
{
	check(InitialEquipmentClasses.Num() <= 3);
	for (int i = 0; i < InitialEquipmentClasses.Num(); ++i)
	{
		if (InitialEquipmentClasses[i])
		{
			AuthAddEquipmentToSlotByClass(InitialEquipmentClasses[i]);
		}
	}
}

void UGunnerEquipmentManagerComponent::RelaseEquipmentManagerComponent()
{
	if (GetOwner()->HasAuthority())
	{
		AuthRemoveAllEquipments();
	}
}

void UGunnerEquipmentManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGunnerEquipmentManagerComponent, NewEquipmentSlots);
	DOREPLIFETIME_CONDITION_NOTIFY(UGunnerEquipmentManagerComponent, CurrentEquippedEquipment, COND_None, REPNOTIFY_Always);
}


void UGunnerEquipmentManagerComponent::AuthAddEquipmentToSlotByClass(TSubclassOf<AGunnerEquipment> EquipmentClass)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	AGunnerEquipment* NewEquipment = GetWorld()->SpawnActor<AGunnerEquipment>(EquipmentClass, SpawnParams);
	NewEquipment->OnAuthAcquired();

	FEquipmentSlot* SlotPtr = NewEquipmentSlots.FindByPredicate([EquipmentClass](const FEquipmentSlot& Slot)
	{
		return Slot.DesiredEquipmentType == EquipmentClass.GetDefaultObject()->GetEquipmentType();
	});

	if (!SlotPtr)
	{
		return;
	}

	if (SlotPtr && SlotPtr->SlottedEquipment)
	{
		SlotPtr->SlottedEquipment->OnAuthLost();
	}

	SlotPtr->SlottedEquipment = NewEquipment;
}

void UGunnerEquipmentManagerComponent::AuthAddEquipment(AGunnerEquipment* NewEquipment)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}
	NewEquipment->SetOwner(GetOwner());
	NewEquipment->OnAuthAcquired();

	FEquipmentSlot* SlotPtr = NewEquipmentSlots.FindByPredicate([Equipment = NewEquipment](const FEquipmentSlot& Slot)
	{
		return Slot.DesiredEquipmentType == Equipment->GetEquipmentType();
	});

	if (!SlotPtr)
	{
		return;
	}

	if (SlotPtr && SlotPtr->SlottedEquipment)
	{
		return;
		SlotPtr->SlottedEquipment->OnAuthLost();
	}

	SlotPtr->SlottedEquipment = NewEquipment;
}

void UGunnerEquipmentManagerComponent::AuthRemoveAllEquipments()
{
	for (FEquipmentSlot& Slot : NewEquipmentSlots)
	{
		if (Slot.SlottedEquipment)
		{
			Slot.SlottedEquipment->Destroy();
		}
	}
	NewEquipmentSlots.Empty();
}

AGunnerEquipment* UGunnerEquipmentManagerComponent::DropCurrentEquipment()
{
	AGunnerEquipment* LastEquipment = nullptr;
	if (CurrentEquippedEquipment)
	{
		FEquipmentSlot* SlotPtr = NewEquipmentSlots.FindByPredicate([Equipment = CurrentEquippedEquipment.Get()](const FEquipmentSlot& Slot)
		{
			return Slot.SlottedEquipment == Equipment;
		});
		SlotPtr->SlottedEquipment = nullptr;

		CurrentEquippedEquipment->OnUnequipped();
		CurrentEquippedEquipment->OnAuthLost();
		CurrentEquippedEquipment->SetOwner(nullptr);
		LastEquipment = CurrentEquippedEquipment;
		CurrentEquippedEquipment = nullptr;
	}
	return LastEquipment;
}


AGunnerEquipment* UGunnerEquipmentManagerComponent::GetCurrentEquippedEquipment() const
{
	return CurrentEquippedEquipment;
}

void UGunnerEquipmentManagerComponent::SetCurrentEquipmentByEquipmentType(EEquipmentType EquipmentType)
{
	FEquipmentSlot* SlotPtr = NewEquipmentSlots.FindByPredicate([EquipmentType](const FEquipmentSlot& Slot)
	{
		return Slot.DesiredEquipmentType == EquipmentType;
	});

	if (SlotPtr && SlotPtr->SlottedEquipment != CurrentEquippedEquipment)
	{
		AGunnerEquipment* LastEquipment = CurrentEquippedEquipment;
		CurrentEquippedEquipment = SlotPtr->SlottedEquipment;
		AActor* ActorOwner = GetOwner();
		if (ActorOwner && ActorOwner->HasAuthority())
		{
			OnRep_CurrentEquippedEquipment(LastEquipment);
		}
	}
}

AGunnerEquipment* UGunnerEquipmentManagerComponent::GetEquipmentByEquipmentType(EEquipmentType EquipmentType) const
{
	const FEquipmentSlot* SlotPtr = NewEquipmentSlots.FindByPredicate([EquipmentType](const FEquipmentSlot& Slot)
	{
		return Slot.DesiredEquipmentType == EquipmentType;
	});

	if (SlotPtr)
	{
		return SlotPtr->SlottedEquipment;
	}
	return nullptr;
}

void UGunnerEquipmentManagerComponent::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y)
{
	AActor* DebugTarget = HUD->GetCurrentDebugTargetActor();
	if (!DebugTarget)
	{
		return;
	}

	if (UGunnerEquipmentManagerComponent* EquipmentManager = DebugTarget->GetComponentByClass<UGunnerEquipmentManagerComponent>())
	{
		EquipmentManager->InternalOnShowDebugInfo(DebugTarget, HUD, Canvas, DebugDisplayInfo, X, Y);
	}
}

void UGunnerEquipmentManagerComponent::InternalOnShowDebugInfo(AActor* Actor, AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y)
{
	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	if (HUD->ShouldDisplayDebug(TEXT("ActionSystem")))
	{
		DisplayDebugManager.SetFont(GEngine->GetTinyFont());
		DisplayDebugManager.SetDrawColor(FColor::Orange);
		DisplayDebugManager.DrawString(FString::Printf(TEXT("Current Equipped Equipment: %s"), CurrentEquippedEquipment ? *CurrentEquippedEquipment->GetName() : TEXT("None")));

		DisplayDebugManager.SetDrawColor(FColor::White);

		if (CurrentEquippedEquipment)
		{
			CurrentEquippedEquipment->OnShowDebugInfo(HUD, Canvas, DebugDisplayInfo, X, Y);
		}
	}
}

void UGunnerEquipmentManagerComponent::OnRep_CurrentEquippedEquipment(AGunnerEquipment* OldEquippedEquipment)
{
	if (OldEquippedEquipment)
	{
		OldEquippedEquipment->OnUnequipped();
	}

	if (CurrentEquippedEquipment)
	{
		CurrentEquippedEquipment->OnEquipped();
	}

	OnEquippedEquipmentChangedDelegate.Broadcast(CurrentEquippedEquipment, OldEquippedEquipment);
}
