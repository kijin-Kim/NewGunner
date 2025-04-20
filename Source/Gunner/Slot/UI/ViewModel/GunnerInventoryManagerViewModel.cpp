// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerInventoryManagerViewModel.h"

#include "Action/NexusActionComponent.h"
#include "Blueprint/UserWidget.h"
#include "Gunner/Slot/GunnerInventoryManagerComponent.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"


void UGunnerInventoryManagerViewModel::OnCreateViewModel(const UUserWidget* UserWidget)
{
	Super::OnCreateViewModel(UserWidget);
	InventoryManagerComponent = UGunnerInventoryManagerComponent::GetInventoryManagerComponentFromActor(UserWidget->GetOwningPlayerPawn());
}
