// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerInventoryManagerComponent.h"

#include "GunnerInventoryManagerInterface.h"
#include "GunnerItem.h"
#include "GunnerItemDef.h"
#include "GunnerSlotItemPlaceholder.h"
#include "Action/NexusActionComponent.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Canvas.h"
#include "GameFramework/HUD.h"
#include "Gunner/Gunner.h"
#include "Gunner/Action/GunnerAction_DropSlotItem.h"
#include "Misc/DataValidation.h"
#include "Net/UnrealNetwork.h"


UGunnerInventoryManagerComponent::UGunnerInventoryManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

#if WITH_EDITOR
EDataValidationResult UGunnerInventoryManagerComponent::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult ValidationResult = Super::IsDataValid(Context);
	TArray<AGunnerItem*> TempItems;
	for (TSubclassOf<AGunnerItem> ItemClass : StartItemClasses)
	{
		if (!ItemClass)
		{
			continue;
		}

		AGunnerItem* DefaultObject = ItemClass->GetDefaultObject<AGunnerItem>();
		if (DefaultObject->CanAcquire(TempItems))
		{
			TempItems.Add(DefaultObject);
			continue;
		}

		ValidationResult = CombineDataValidationResults(ValidationResult, EDataValidationResult::NotValidated);
		Context.AddError(FText::Format(NSLOCTEXT("Gunner", "InvalidStartItemClass", "[{0}]가 인벤토리에 추가될 수 없습니다"), ItemClass->GetDisplayNameText()));
	}
	return ValidationResult;
}
#endif

void UGunnerInventoryManagerComponent::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& YL, float& YPos)
{
	AActor* DebugTarget = HUD->GetCurrentDebugTargetActor();
	if (!DebugTarget)
	{
		return;
	}

	if (UGunnerInventoryManagerComponent* InventoryManager = GetInventoryManagerComponentFromActor(DebugTarget))
	{
		InventoryManager->InternalOnShowDebugInfo(DebugTarget, HUD, Canvas, DebugDisplayInfo, YL, YPos);
	}
}

void UGunnerInventoryManagerComponent::InternalOnShowDebugInfo(AActor* DebugTarget, AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& YL, float& YPos)
{
	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	if (HUD->ShouldDisplayDebug(TEXT("SlotSystem")))
	{
		DisplayDebugManager.SetFont(GEngine->GetTinyFont());
		DisplayDebugManager.SetDrawColor(FColor::Orange);
		for (AGunnerItem* SlotItem : Items)
		{
			if (SlotItem)
			{
				SlotItem->OnShowDebugInfo(HUD, Canvas, DebugDisplayInfo, YL, YPos);
			}
		}
	}
}

UGunnerInventoryManagerComponent* UGunnerInventoryManagerComponent::GetInventoryManagerComponentFromActor(const AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}

	if (UGunnerInventoryManagerComponent* InventoryManager = Actor->GetComponentByClass<UGunnerInventoryManagerComponent>())
	{
		return InventoryManager;
	}

	if (const IGunnerInventoryManagerInterface* InventoryManagerInterface = Cast<IGunnerInventoryManagerInterface>(Actor))
	{
		return InventoryManagerInterface->GetInventoryManagerComponent();
	}

	return nullptr;
}

void UGunnerInventoryManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGunnerInventoryManagerComponent, Items);
}

void UGunnerInventoryManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetOwner());
	check(ActionComponent);

	if (!DropSlotItemActionClass)
	{
		DropSlotItemActionClass = UGunnerAction_DropSlotItem::StaticClass();
	}

	ActionComponent->CallOrAddSetupCompletedDelegate(FOnNexusActionComponentSetupCompletedSignature::FDelegate::CreateWeakLambda(this, [this]()
	{
		UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetOwner());
		check(ActionComponent);

		if (GetOwner()->HasAuthority())
		{
			for (const UGunnerItemDef* ItemDef : StartItemDefs)
			{
				if (ItemDef && ItemDef->ItemClass)
				{
					AGunnerItem* NewItem = GetWorld()->SpawnActorDeferred<AGunnerItem>(ItemDef->ItemClass, FTransform::Identity);
					check(NewItem);
					NewItem->InitializeItem(ItemDef);
					NewItem->FinishSpawning(FTransform::Identity);
					AuthAddItem(NewItem);
				}
			}

			// 근접무기의 플레이스홀더를 인벤토리에 추가합니다. 이미 근접무기가 있을 경우 추가되지 않습니다
			AGunnerItem* Placeholder = GetWorld()->SpawnActor<AGunnerItem>(AGunnerEquippable_MeleePlaceholder::StaticClass());
			check(Placeholder);
			AuthAddItem(Placeholder);
		}
		if (ActionComponent->IsAgentLocallyPlayerControlled())
		{
			for (TSubclassOf<UUserWidget> WidgetClass : InventoryWidgetClasses)
			{
				if (WidgetClass)
				{
					UUserWidget* Widget = CreateWidget<UUserWidget>(Cast<APlayerController>(ActionComponent->GetController()), WidgetClass);
					check(Widget);
					InventoryWidgets.Add(Widget);
					Widget->AddToViewport();
				}
			}
		}

		for (AGunnerItem* Item : PendingRemoves)
		{
			if (Item)
			{
				OnItemRemoved(Item);
			}
		}

		for (AGunnerItem* Item : PendingAdds)
		{
			if (Item)
			{
				OnItemAcquired(Item);
			}
		}

		PendingRemoves.Empty();
		PendingAdds.Empty();
	}));
}

void UGunnerInventoryManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetOwner()))
	{
		ActionComponent->RemoveSetupCompletedDelegate(this);
		if (ActionComponent->IsAgentLocallyPlayerControlled())
		{
			for (UUserWidget* Widget : InventoryWidgets)
			{
				if (Widget)
				{
					Widget->RemoveFromParent();
				}
			}
			InventoryWidgets.Empty();
		}
	}
	Items.Empty();
}


bool UGunnerInventoryManagerComponent::CanAcquireItem(AGunnerItem* Item) const
{
	return Item && Item->CanAcquire(Items);
}

void UGunnerInventoryManagerComponent::AuthAddItem(AGunnerItem* Item)
{
	check(Item);
	AActor* ActorOwner = GetOwner();
	check(ActorOwner && ActorOwner->HasAuthority());
	if (Item->CanAcquire(Items))
	{
		UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetOwner());
		check(ActionComponent);
		Items.Add(Item);
		OnItemAcquired(Item);
	}
}

void UGunnerInventoryManagerComponent::AuthRemoveItem(AGunnerItem* Item, bool bDestroyItem)
{
	check(Item);
	AActor* ActorOwner = GetOwner();
	check(ActorOwner);
	check(ActorOwner->HasAuthority());
	OnItemRemoved(Item);
	if (bDestroyItem)
	{
		Item->Destroy();
	}
	check(Items.Remove(Item) > 0);
}


bool UGunnerInventoryManagerComponent::HasItem(AGunnerItem* Item) const
{
	return Items.Contains(Item);
}

void UGunnerInventoryManagerComponent::OnItemAcquired(AGunnerItem* Item)
{
	check(Item);
	GR_VLOG_SUB(GetAgentActorChecked(), LogGunnerInventory, Display, TEXT("인벤토리 아이템 추가: Item=%s"), *Item->ToString());
	Item->OnAcquired(GetAgentActorChecked());
	Item->PostOnAcquired();

	OnItemAcquiredDelegate.Broadcast(Item);
}

void UGunnerInventoryManagerComponent::OnItemRemoved(AGunnerItem* Item)
{
	check(Item);
	Item->OnRemoved();
	OnItemRemovedDelegate.Broadcast(Item);
	GR_VLOG_SUB(GetAgentActorChecked(), LogGunnerInventory, Display, TEXT("인벤토리 아이템 제거: Item=%s"), *Item->ToString());
}


AActor* UGunnerInventoryManagerComponent::GetAgentActorChecked() const
{
	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetOwner());
	check(ActionComponent);
	AActor* AgentActor = ActionComponent->GetAgentActor();
	check(AgentActor);
	return AgentActor;
}


void UGunnerInventoryManagerComponent::OnRep_Items(const TArray<AGunnerItem*>& OldItems)
{
	TArray<AGunnerItem*> RemovedItems;
	TArray<AGunnerItem*> AddedItems;


	for (AGunnerItem* Item : OldItems)
	{
		if (Item && !Items.Contains(Item))
		{
			RemovedItems.Add(Item);
		}
	}

	for (AGunnerItem* Item : Items)
	{
		if (Item && !OldItems.Contains(Item))
		{
			AddedItems.Add(Item);
		}
	}

	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetOwner());
	check(ActionComponent);
	if (!ActionComponent->IsSetupCompleted())
	{
		PendingRemoves = MoveTemp(RemovedItems);
		PendingAdds = MoveTemp(AddedItems);
		return;
	}


	for (AGunnerItem* Item : RemovedItems)
	{
		if (Item)
		{
			OnItemRemoved(Item);
		}
	}

	for (AGunnerItem* Item : AddedItems)
	{
		if (Item)
		{
			OnItemAcquired(Item);
		}
	}
}
