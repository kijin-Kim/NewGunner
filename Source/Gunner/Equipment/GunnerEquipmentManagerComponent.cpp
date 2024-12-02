// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerEquipmentManagerComponent.h"

#include "DisplayDebugHelpers.h"
#include "GunnerEquipment.h"
#include "Engine/Canvas.h"
#include "GameFramework/HUD.h"
#include "Gunner/Gunner.h"
#include "Gunner/_Core/Event/GunnerEventManagerComponent.h"
#include "Gunner/_Core/Input/GunnerEventMessage.h"
#include "Net/UnrealNetwork.h"


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

void UGunnerEquipmentManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGunnerEquipmentManagerComponent, EquipmentSlots);
	DOREPLIFETIME(UGunnerEquipmentManagerComponent, CurrentEquippedEquipment);
}

void UGunnerEquipmentManagerComponent::AuthAddEquipmentToSlot(int32 SlotIndex, TSubclassOf<AGunnerEquipment> EquipmentClass)
{
	if (!EquipmentSlots.IsValidIndex(SlotIndex))
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

void UGunnerEquipmentManagerComponent::SetCurrentEquipmentByIndex(int32 SlotIndex)
{
	if (EquipmentSlots.IsValidIndex(SlotIndex) && EquipmentSlots[SlotIndex] != CurrentEquippedEquipment)
	{
		AGunnerEquipment* LastEquipment = CurrentEquippedEquipment;
		CurrentEquippedEquipment = EquipmentSlots[SlotIndex];
		OnRep_CurrentEquippedEquipment(LastEquipment);
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

void UGunnerEquipmentManagerComponent::ServerFireHitScan_Implementation(const TArray<FClientHitScanData>& ClientHitScanData)
{
	if (!ClientHitScanData.IsEmpty())
	{
		DrawDebugLine(GetWorld(), ClientHitScanData[0].ShooterLocation, ClientHitScanData[0].ShooterLocation + ClientHitScanData[0].ShooterRotation.Vector() * 10000.0f, FColor::Green, false, 2.0f, 0, 3.0f);
	}

	TArray<AActor*> AlreadyHitActors;
	for (const FClientHitScanData& HitScanData : ClientHitScanData)
	{
		DrawDebugSphere(GetWorld(), HitScanData.HitLocation, 10.0f, 12, FColor::Green, false, 2.0f, 0, 3.0f);
		if (HitScanData.HitActor && AlreadyHitActors.Find(HitScanData.HitActor) == INDEX_NONE)
		{
			AlreadyHitActors.Add(HitScanData.HitActor);
			if (UGunnerEventManagerComponent* EventManagerComponent = UGunnerEventManagerComponent::GetEventManagerComponentFromActor(HitScanData.HitActor))
			{
				FGunnerEventMessage HitScanMessage;
				HitScanMessage.Instigator = CurrentEquippedEquipment;
				EventManagerComponent->SendEventToActor(FGameplayTag::RequestGameplayTag(FName("GameEvent.Damaged")), HitScanMessage, HitScanData.HitActor);
			}
		}
	}
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
