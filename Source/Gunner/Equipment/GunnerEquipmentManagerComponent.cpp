// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerEquipmentManagerComponent.h"

#include "GunnerEquipment.h"
#include "GunnerEquipmentDef.h"
#include "Engine/Canvas.h"
#include "GameFramework/HUD.h"
#include "Gunner/Gunner.h"

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

	EquipmentSlots.SetNum(static_cast<uint8>(EEquipmentType::EquipmentTypeCount));
}

void UGunnerEquipmentManagerComponent::AuthInitEquipmentManagerComponent()
{
	if (!ensure(GetOwner()->HasAuthority()))
	{
		return;
	}

	for (TObjectPtr<UGunnerEquipmentDef> EquipmentDef : InitialEquipmentDefs)
	{
		if (EquipmentDef)
		{
			AuthAddEquipmentByEquipmentDef(EquipmentDef);
		}
	}
}

void UGunnerEquipmentManagerComponent::AuthRelaseEquipmentManagerComponent()
{
	if (ensure(GetOwner()->HasAuthority()))
	{
		AuthRemoveAllEquipments();
	}
}

void UGunnerEquipmentManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGunnerEquipmentManagerComponent, EquipmentSlots);
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

	FEquipmentSlot* SlotPtr = EquipmentSlots.FindByPredicate([EquipmentClass](const FEquipmentSlot& Slot)
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
	SlotPtr->SlottedEquipment->SetMeshVisibility(false);
}

void UGunnerEquipmentManagerComponent::AuthAddEquipment(AGunnerEquipment* NewEquipment)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}
	NewEquipment->SetOwner(GetOwner());
	NewEquipment->OnAuthAcquired();

	FEquipmentSlot* SlotPtr = EquipmentSlots.FindByPredicate([Equipment = NewEquipment](const FEquipmentSlot& Slot)
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
	for (FEquipmentSlot& Slot : EquipmentSlots)
	{
		if (Slot.SlottedEquipment)
		{
			Slot.SlottedEquipment->Destroy();
		}
	}
	EquipmentSlots.Empty();
}

void UGunnerEquipmentManagerComponent::AuthAddEquipmentByEquipmentDef(UGunnerEquipmentDef* EquipmentDef)
{
	if (!ensure(GetOwner()->HasAuthority()))
	{
		GR_LOG_SUB_FN(LogGunner, Warning, TEXT("함수는 서버에서만 호출 가능합니다."));
		return;
	}
	check(EquipmentDef);
	check(EquipmentDef->EquipmentClass);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();

	AGunnerEquipment* NewEquipment = GetWorld()->SpawnActor<AGunnerEquipment>(EquipmentDef->EquipmentClass, SpawnParams);
	NewEquipment->SetEquipmentDef(EquipmentDef);
	NewEquipment->OnAuthAcquired();

	FEquipmentSlot* SlotPtr = EquipmentSlots.FindByPredicate([NewEquipment](const FEquipmentSlot& Slot)
	{
		return Slot.DesiredEquipmentType == NewEquipment->GetEquipmentType();
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
	SlotPtr->SlottedEquipment->SetMeshVisibility(false);
}


AGunnerEquipment* UGunnerEquipmentManagerComponent::DropCurrentEquipment()
{
	AGunnerEquipment* LastEquipment = nullptr;
	if (CurrentEquippedEquipment)
	{
		FEquipmentSlot* SlotPtr = EquipmentSlots.FindByPredicate([Equipment = CurrentEquippedEquipment.Get()](const FEquipmentSlot& Slot)
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
	FEquipmentSlot* SlotPtr = EquipmentSlots.FindByPredicate([EquipmentType](const FEquipmentSlot& Slot)
	{
		return Slot.DesiredEquipmentType == EquipmentType;
	});

	if (SlotPtr && SlotPtr->SlottedEquipment != CurrentEquippedEquipment)
	{
		AGunnerEquipment* LastEquipment = CurrentEquippedEquipment;
		CurrentEquippedEquipment = SlotPtr->SlottedEquipment;
		OnRep_CurrentEquippedEquipment(LastEquipment);
	}
}

AGunnerEquipment* UGunnerEquipmentManagerComponent::GetEquipmentByEquipmentType(EEquipmentType EquipmentType) const
{
	const FEquipmentSlot* SlotPtr = EquipmentSlots.FindByPredicate([EquipmentType](const FEquipmentSlot& Slot)
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
	for (const FEquipmentSlot& Slot : EquipmentSlots)
	{
		if (Slot.SlottedEquipment && (Slot.SlottedEquipment != CurrentEquippedEquipment))
		{
			Slot.SlottedEquipment->SetMeshVisibility(false);
		}
	}

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

void UGunnerEquipmentManagerComponent::OnRep_EquipmentSlots()
{
	for (const FEquipmentSlot& Slot : EquipmentSlots)
	{
		if (Slot.SlottedEquipment && (Slot.SlottedEquipment != CurrentEquippedEquipment))
		{
			Slot.SlottedEquipment->SetMeshVisibility(false);
		}
	}
}
