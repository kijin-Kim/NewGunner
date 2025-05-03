// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerInventoryViewModelBase.h"

#include "Action/NexusActionComponent.h"
#include "Blueprint/UserWidget.h"
#include "Gunner/Item/GunnerInventoryManagerComponent.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"


void UGunnerInventoryViewModelBase::OnCreateViewModel(const UUserWidget* UserWidget)
{
	Super::OnCreateViewModel(UserWidget);
	InventoryManagerComponent = UGunnerInventoryManagerComponent::GetInventoryManagerComponentFromActor(UserWidget->GetOwningPlayerPawn());
}
