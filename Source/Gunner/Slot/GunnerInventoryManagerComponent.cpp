// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerInventoryManagerComponent.h"

#include "GunnerInventoryManagerInterface.h"
#include "GunnerItem.h"
#include "GunnerSlotItemPlaceholder.h"
#include "Action/NexusActionComponent.h"
#include "Engine/Canvas.h"
#include "GameFramework/HUD.h"
#include "Gunner/Gunner.h"
#include "Gunner/Action/GunnerActionDropSlotItem.h"
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

void UGunnerInventoryManagerComponent::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y)
{
	AActor* DebugTarget = HUD->GetCurrentDebugTargetActor();
	if (!DebugTarget)
	{
		return;
	}

	if (UGunnerInventoryManagerComponent* InventoryManager = GetInventoryManagerComponentFromActor(DebugTarget))
	{
		InventoryManager->InternalOnShowDebugInfo(DebugTarget, HUD, Canvas, DebugDisplayInfo, X, Y);
	}
}

void UGunnerInventoryManagerComponent::InternalOnShowDebugInfo(AActor* DebugTarget, AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y)
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
				SlotItem->OnShowDebugInfo(HUD, Canvas, DebugDisplayInfo, X, Y);
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
		DropSlotItemActionClass = UGunnerActionDropSlotItem::StaticClass();
	}

	ActionComponent->CallOrAddSetupCompletedDelegate(FOnNexusActionComponentSetupCompletedSignature::FDelegate::CreateWeakLambda(this, [this]()
	{
		UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetOwner());
		check(ActionComponent);

		if (GetOwner()->HasAuthority())
		{
			ActionComponent->AuthAddAction(DropSlotItemActionClass);
			ActionComponent->AuthAddAction(UGunnerActionCycleSlotItem::StaticClass());

			for (TSubclassOf<AGunnerItem> ItemClass : StartItemClasses)
			{
				if (ItemClass)
				{
					AGunnerItem* NewItem = GetWorld()->SpawnActor<AGunnerItem>(ItemClass);
					check(NewItem);
					AuthAddItem(NewItem);
				}
			}

			// 근접무기의 플레이스홀더를 인벤토리에 추가합니다. 이미 근접무기가 있을 경우 추가되지 않습니다
			AGunnerItem* Placeholder = GetWorld()->SpawnActor<AGunnerItem>(AGunnerEquippable_MeleePlaceholder::StaticClass());
			check(Placeholder);
			AuthAddItem(Placeholder);
		}
	}));
}

void UGunnerInventoryManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetOwner()))
	{
		ActionComponent->RemoveSetupCompletedDelegate(this);
	}
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
		GR_VLOG_SUB(ActorOwner, LogGunner, Log, TEXT("아이템 [%s]가 인벤토리에 추가되었습니다"), *Item->GetName());
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
	GR_VLOG_SUB(ActorOwner, LogGunner, Log, TEXT("아이템 [%s]가 인벤토리에서 제거되었습니다"), *Item->GetName());
}

bool UGunnerInventoryManagerComponent::HasItem(AGunnerItem* Item) const
{
	return Items.Contains(Item);
}

void UGunnerInventoryManagerComponent::OnItemAcquired(AGunnerItem* Item)
{
	check(Item);
	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetOwner());
	Item->OnAcquired(ActionComponent->GetAgentActor());
	Item->PostOnAcquired();
	OnItemAcquiredDelegate.Broadcast(Item);
}

void UGunnerInventoryManagerComponent::OnItemRemoved(AGunnerItem* Item)
{
	check(Item);
	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetOwner());
	Item->OnRemoved();
	OnItemRemovedDelegate.Broadcast(Item);
}

void UGunnerInventoryManagerComponent::OnRep_Items(const TArray<AGunnerItem*>& OldItems)
{
	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetOwner());
	check(ActionComponent);
	if (!ActionComponent->IsSetupCompleted())
	{
		return;
	}

	for (AGunnerItem* Item : OldItems)
	{
		if (Item && !Items.Contains(Item))
		{
			OnItemRemoved(Item);
		}
	}

	for (AGunnerItem* Item : Items)
	{
		if (Item && !OldItems.Contains(Item))
		{
			OnItemAcquired(Item);
		}
	}
}
