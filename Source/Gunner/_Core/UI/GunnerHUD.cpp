// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerHUD.h"

#include "EngineUtils.h"
#include "GunnerUserWidget.h"
#include "MVVMSubsystem.h"
#include "VMSlotItem.h"
#include "Gunner/_Core/GunnerOverlayWidget.h"
#include "Action/NexusActionComponent.h"
#include "Gunner/Slot/GunnerSlotManagerComponent.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"
#include "Gunner/_Core/UI/GunnerOverlayWidgetController.h"
#include "View/MVVMView.h"


void AGunnerHUD::SetupHUD(APlayerState* PlayerState)
{
	check(OverlayWidgetClass);
	if (!OverlayWidget)
	{
		OverlayWidget = CreateWidget<UGunnerOverlayWidget>(GetOwningPlayerController(), OverlayWidgetClass);
		UGunnerOverlayWidgetController* OverlayWidgetController = NewObject<UGunnerOverlayWidgetController>(GetOwningPlayerController());
		OverlayWidgetController->InitWidgetController(PlayerState);
		OverlayWidget->InitUserWidget(OverlayWidgetController);
		OverlayWidget->AddToViewport();
	}

	APawn* Pawn = GetOwningPlayerController()->GetPawn();
	check(Pawn);
	if (Pawn)
	{
		UGunnerSlotManagerComponent* SlotManagerComponent = UGunnerSlotManagerComponent::GetSlotManagerComponentFromActor(Pawn);
		check(SlotManagerComponent);
		if (SlotManagerComponent)
		{
			SlotManagerComponent->OnGunnerSlotItemAcquiredDelegate.AddDynamic(this, &AGunnerHUD::OnSlotItemAcquired);
			SlotManagerComponent->OnGunnerSlotItemRemovedDelegate.AddDynamic(this, &AGunnerHUD::OnSlotItemRemoved);
		}

		UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(Pawn);
		check(ActionComponent);
		UNexusProperty* SlotIndexProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_SlotIndex);
		check(SlotIndexProperty);
		SlotIndexProperty->OnDirtyDelegate.AddDynamic(this, &AGunnerHUD::HandleSlotIndexDirty);
	}
}

void AGunnerHUD::GetDebugActorList(TArray<AActor*>& InOutList)
{
	Super::GetDebugActorList(InOutList);
	InOutList.RemoveAll([this](AActor* Actor)
	{
		return !UNexusActionComponent::GetActionComponentFromActor(Actor);
	});
	UWorld* World = GetWorld();
	check(World);
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (IsValid(Actor) && UNexusActionComponent::GetActionComponentFromActor(Actor) != nullptr)
		{
			InOutList.AddUnique(Actor);
		}
	}
}

void AGunnerHUD::OnSlotItemAcquired(AGunnerSlotItem* Item)
{
	TSubclassOf<UUserWidget> WidgetClass = Item->GetSlotItemWidgetClass();
	if (!WidgetClass)
	{
		return;
	}

	TObjectPtr<UUserWidget>& SlotWidget = SlotWidgets.FindOrAdd(Item->GetSlotType());
	if (!SlotWidget)
	{
		SlotWidget = CreateWidget<UUserWidget>(GetOwningPlayerController(), WidgetClass);
	}

	SlotWidget->AddToViewport();
}

void AGunnerHUD::OnSlotItemRemoved(AGunnerSlotItem* Item)
{
	TObjectPtr<UUserWidget>* SlotWidgetPtr = SlotWidgets.Find(Item->GetSlotType());
	if (SlotWidgetPtr && *SlotWidgetPtr)
	{
		(*SlotWidgetPtr)->RemoveFromParent();
	}
}

void AGunnerHUD::HandleSlotIndexDirty(float OldValue, float NewValue)
{
	APawn* Pawn = GetOwningPlayerController()->GetPawn();
	UGunnerSlotManagerComponent* SlotManagerComponent = UGunnerSlotManagerComponent::GetSlotManagerComponentFromActor(Pawn);
	check(SlotManagerComponent);

	if (!SlotManagerComponent->IsSlotEmpty(static_cast<EGunnerSlotType>(OldValue)))
	{
		SetSlotWidgetVisibility(static_cast<EGunnerSlotType>(OldValue), ESlateVisibility::Hidden);
	}

	if (!SlotManagerComponent->IsSlotEmpty(static_cast<EGunnerSlotType>(NewValue)))
	{
		SetSlotWidgetVisibility(static_cast<EGunnerSlotType>(NewValue), ESlateVisibility::Visible);
	}
}

void AGunnerHUD::SetSlotWidgetVisibility(EGunnerSlotType SlotType, ESlateVisibility Visibility)
{
	TObjectPtr<UUserWidget>* SlotWidgetPtr = SlotWidgets.Find(SlotType);
	if (SlotWidgetPtr && *SlotWidgetPtr)
	{
		(*SlotWidgetPtr)->SetVisibility(Visibility);
	}
}
