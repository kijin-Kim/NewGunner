// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerHUD.h"

#include "EngineUtils.h"
#include "GunnerUserWidget.h"
#include "Gunner/_Core/GunnerOverlayWidget.h"
#include "Action/NexusActionComponent.h"
#include "Gunner/Item/GunnerInventoryManagerComponent.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"
#include "Gunner/_Core/UI/GunnerOverlayWidgetController.h"




void AGunnerHUD::GetDebugActorList(TArray<AActor*>& InOutList)
{
	UWorld* World = GetWorld();
	check(World);
	InOutList.Empty();
	TArray<UNexusActionComponent*> ActionComponents;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (UNexusActionComponent* ActionComponent = Actor->GetComponentByClass<UNexusActionComponent>())
		{
			InOutList.Add(ActionComponent->GetAgentActor() ? ActionComponent->GetAgentActor() : Actor);
		}
	}
}

void AGunnerHUD::OnSlotItemActivated(EGunnerSlotType SlotType)
{
	// UGunnerInventoryManagerComponent* SlotManagerComponent = GetInventoryManagerComponentChecked();
	// AGunnerSlotItem* Item = SlotManagerComponent->GetSlotItemByType(SlotType);
	// check(Item);
	// if (!Item)
	// {
	// 	return;
	// }
	//
	// const UGunnerSlotItemUiData* UIData = Item->GetSlotItemUIData();
	// if (!UIData)
	// {
	// 	return;
	// }
	//
	// for (const UGunnerSlotItemTransientUiComponentBase* UIComponent : UIData->TransientComponents)
	// {
	// 	TSubclassOf<UUserWidget> WidgetClass = UIComponent->WidgetClass;
	// 	if (!WidgetClass)
	// 	{
	// 		continue;
	// 	}
	//
	// 	FGunnerSlotWidgetContainer& SlotWidgets = SlotTypeWidgetMap.FindOrAdd(Item->GetSlotType());
	// 	check(SlotWidgets.Widgets.IsEmpty());
	// 	int32 Index = SlotWidgets.Widgets.Add(CreateWidget<UUserWidget>(GetOwningPlayerController(), WidgetClass));
	// 	SlotWidgets.Widgets[Index]->AddToViewport();
	// }
}

void AGunnerHUD::OnSlotItemDeactivated(EGunnerSlotType SlotType)
{
	// if (FGunnerSlotWidgetContainer* SlotWidgetsPtr = SlotTypeWidgetMap.Find(SlotType))
	// {
	// 	for (UUserWidget* Widget : SlotWidgetsPtr->Widgets)
	// 	{
	// 		if (Widget && Widget->IsInViewport())
	// 		{
	// 			Widget->RemoveFromParent();
	// 		}
	// 	}
	// 	SlotWidgetsPtr->Widgets.Empty();
	// }
}

void AGunnerHUD::HandleSlotIndexDirty(float OldValue, float NewValue)
{
	OnSlotItemDeactivated(static_cast<EGunnerSlotType>(OldValue));
	OnSlotItemActivated(static_cast<EGunnerSlotType>(NewValue));
}

UGunnerInventoryManagerComponent* AGunnerHUD::GetInventoryManagerComponentChecked() const
{
	APlayerController* PC = GetOwningPlayerController();
	check(PC);
	APawn* Pawn = PC->GetPawn();
	check(Pawn);
	UGunnerInventoryManagerComponent* InventoryManagerComponent = UGunnerInventoryManagerComponent::GetInventoryManagerComponentFromActor(Pawn);
	check(InventoryManagerComponent);
	return InventoryManagerComponent;
}
