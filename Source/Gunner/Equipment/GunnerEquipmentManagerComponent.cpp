// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerEquipmentManagerComponent.h"

#include "GunnerEquipment.h"
#include "Camera/CameraComponent.h"
#include "Engine/Canvas.h"
#include "GameFramework/Character.h"
#include "GameFramework/HUD.h"
#include "Gunner/_Core/LagCompensationComponent.h"
#include "Gunner/_Core/Event/GunnerEventManagerComponent.h"
#include "Gunner/_Core/Input/GunnerEventMessage.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/SphylElem.h"


// Sets default values for this component's properties
UGunnerEquipmentManagerComponent::UGunnerEquipmentManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	EquipmentSlots.SetNum(MaxSlots);
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
			AuthAddEquipmentToSlot(i, InitialEquipmentClasses[i]);
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
	DOREPLIFETIME_CONDITION_NOTIFY(UGunnerEquipmentManagerComponent, EquipmentSlots, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGunnerEquipmentManagerComponent, CurrentEquippedEquipment, COND_None, REPNOTIFY_Always);
}

void UGunnerEquipmentManagerComponent::AuthAddEquipmentToSlot(int32 SlotIndex, TSubclassOf<AGunnerEquipment> EquipmentClass)
{
	if (!EquipmentSlots.IsValidIndex(SlotIndex) || !GetOwner()->HasAuthority())
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	AGunnerEquipment* NewEquipment = GetWorld()->SpawnActor<AGunnerEquipment>(EquipmentClass, SpawnParams);
	NewEquipment->OnAcquired();

	if (EquipmentSlots[SlotIndex])
	{
		EquipmentSlots[SlotIndex]->OnLost();
	}

	EquipmentSlots[SlotIndex] = NewEquipment;
}

void UGunnerEquipmentManagerComponent::AuthRemoveAllEquipments()
{
	for (AGunnerEquipment* Equipment : EquipmentSlots)
	{
		if (Equipment)
		{
			Equipment->Destroy();
		}
	}
	EquipmentSlots.Empty();
}

void UGunnerEquipmentManagerComponent::DropCurrentEquipment()
{
	if (CurrentEquippedEquipment)
	{
		CurrentEquippedEquipment->OnUnequipped();
		CurrentEquippedEquipment->OnLost();
		TObjectPtr<AGunnerEquipment>* CurrentPtr = EquipmentSlots.FindByPredicate([this](AGunnerEquipment* Equipment)
		{
			return Equipment == CurrentEquippedEquipment;
		});
		*CurrentPtr = nullptr;
		CurrentEquippedEquipment = nullptr;
	}
	
}

void UGunnerEquipmentManagerComponent::SetCurrentEquipmentByIndex(int32 SlotIndex)
{
	if (EquipmentSlots.IsValidIndex(SlotIndex) && EquipmentSlots[SlotIndex] != CurrentEquippedEquipment)
	{
		AGunnerEquipment* LastEquipment = CurrentEquippedEquipment;
		CurrentEquippedEquipment = EquipmentSlots[SlotIndex];
		AActor* ActorOwner = GetOwner();
		if (ActorOwner && ActorOwner->HasAuthority())
		{
			OnRep_CurrentEquippedEquipment(LastEquipment);
		}
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

AGunnerEquipment* UGunnerEquipmentManagerComponent::GetCurrentEquippedEquipment() const
{
	return CurrentEquippedEquipment;
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

void UGunnerEquipmentManagerComponent::OnRep_EquipmentSlots(const TArray<AGunnerEquipment*>& OldEquipmentSlots)
{
	for (AGunnerEquipment* Equipment : EquipmentSlots)
	{
		if (Equipment && !OldEquipmentSlots.Contains(Equipment))
		{
			Equipment->OnAcquired();
		}
	}

	for (AGunnerEquipment* Equipment : OldEquipmentSlots)
	{
		if (Equipment && !EquipmentSlots.Contains(Equipment))
		{
			Equipment->OnLost();
		}
	}
}
