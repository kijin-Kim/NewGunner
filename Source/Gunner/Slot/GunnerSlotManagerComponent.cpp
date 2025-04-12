// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerSlotManagerComponent.h"

#include "GunnerSlotManagerInterface.h"
#include "NexusActionComponent.h"
#include "Action/NexusAction.h"
#include "Action/NexusActionDef.h"
#include "Engine/Canvas.h"
#include "GameFramework/HUD.h"
#include "Gunner/Gunner.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"
#include "Misc/DataValidation.h"
#include "Net/UnrealNetwork.h"

AGunnerSlotItem::AGunnerSlotItem()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AGunnerSlotItem::AuthAddPersistentActivationActions()
{
	check(HasAuthority());
	AuthAddDesiredActions(PersistentActivationActions, PersistentActivationActionHandles);
}

void AGunnerSlotItem::AuthAddTransientActivationActions()
{
	check(HasAuthority());
	AuthAddDesiredActions(TransientActivationActions, TransientActivationActionHandles);
}

void AGunnerSlotItem::AuthRemovePersistentActivationActions()
{
	check(HasAuthority());
	AuthRemoveDesiredActions(PersistentActivationActionHandles);
}

void AGunnerSlotItem::AuthRemoveTransientActivationActions()
{
	check(HasAuthority());
	AuthRemoveDesiredActions(TransientActivationActionHandles);
}

UNexusActionComponent* AGunnerSlotItem::GetActionComponent() const
{
	AActor* ActorOwner = GetOwner();
	return ActorOwner ? UNexusActionComponent::GetActionComponentFromActor(ActorOwner) : nullptr;
}

void AGunnerSlotItem::AuthAddDesiredActions(const TArray<TSubclassOf<UNexusAction>>& ActionsToAdd, TArray<FNexusActionDefHandle>& AddedActionHandles)
{
	check(HasAuthority());
	UNexusActionComponent* ActionComponent = GetActionComponent();
	if (ActionComponent)
	{
		for (auto ActionClass : ActionsToAdd)
		{
			if (ActionClass)
			{
				FNexusActionDef ActionDef(this, ActionClass);
				FNexusActionDefHandle AddedHandle = ActionComponent->AuthAddAction(ActionDef);
				AddedActionHandles.Add(AddedHandle);
			}
		}
	}
}

void AGunnerSlotItem::AuthRemoveDesiredActions(TArray<FNexusActionDefHandle>& AddedActionHandles)
{
	check(HasAuthority())
	UNexusActionComponent* ActionComponent = GetActionComponent();
	check(ActionComponent);

	for (auto& ActionHandle : AddedActionHandles)
	{
		ActionComponent->AuthRemoveAction(ActionHandle);
	}
	AddedActionHandles.Empty();
}

UGunnerSlotManagerComponent::UGunnerSlotManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	SlotItems.SetNum(static_cast<int>(EGunnerSlotType::Num));
}

#if WITH_EDITOR
EDataValidationResult UGunnerSlotManagerComponent::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult ValidationResult = Super::IsDataValid(Context);
	for (int i = 0; i < static_cast<int>(EGunnerSlotType::Num); ++i)
	{
		if (StartItemClasses[i] && StartItemClasses[i]->GetDefaultObject<AGunnerSlotItem>()->GetSlotType() != static_cast<EGunnerSlotType>(i))
		{
			ValidationResult = CombineDataValidationResults(ValidationResult, EDataValidationResult::NotValidated);
			Context.AddError(FText::Format(NSLOCTEXT("Gunner", "InvalidStartItemClass", "StartItemClasses[{0}]의 슬롯 타입이 일치하지 않습니다."), i));
		}
	}
	return ValidationResult;
}
#endif

void UGunnerSlotManagerComponent::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y)
{
	AActor* DebugTarget = HUD->GetCurrentDebugTargetActor();
	if (!DebugTarget)
	{
		return;
	}

	if (UGunnerSlotManagerComponent* SlotManager = GetSlotManagerComponentFromActor(DebugTarget))
	{
		SlotManager->InternalOnShowDebugInfo(DebugTarget, HUD, Canvas, DebugDisplayInfo, X, Y);
	}
}

void UGunnerSlotManagerComponent::InternalOnShowDebugInfo(AActor* Actor, AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y)
{
	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	if (HUD->ShouldDisplayDebug(TEXT("SlotSystem")))
	{
		DisplayDebugManager.SetFont(GEngine->GetTinyFont());
		DisplayDebugManager.SetDrawColor(FColor::Orange);
		DisplayDebugManager.DrawString(FString::Printf(TEXT("선택된 슬롯: %s"), *UEnum::GetValueAsString(GetCurrentSlotType())));
		for (AGunnerSlotItem* SlotItem : SlotItems)
		{
			if (SlotItem)
			{
				DisplayDebugManager.DrawString(FString::Printf(TEXT("슬롯 [%s] 아이템: %s"), *UEnum::GetValueAsString(SlotItem->GetSlotType()), *SlotItem->GetName()));
			}
		}
	}
}

UGunnerSlotManagerComponent* UGunnerSlotManagerComponent::GetSlotManagerComponentFromActor(const AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}

	if (UGunnerSlotManagerComponent* SlotManagerComponent = Actor->GetComponentByClass<UGunnerSlotManagerComponent>())
	{
		return SlotManagerComponent;
	}

	if (const IGunnerSlotManagerInterface* SlotManagerInterface = Cast<IGunnerSlotManagerInterface>(Actor))
	{
		return SlotManagerInterface->GetSlotManagerComponent();
	}

	return nullptr;
}

void UGunnerSlotManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGunnerSlotManagerComponent, SlotItems);
}

void UGunnerSlotManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetOwner());
	check(ActionComponent);
	ActionComponent->CallOrAddSetupCompletedDelegate(FOnNexusActionComponentSetupCompletedSignature::FDelegate::CreateWeakLambda(this, [this]()
	{
		UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetOwner());
		check(ActionComponent);

		if (GetOwner()->HasAuthority())
		{
			for (TSubclassOf<AGunnerSlotItem> ItemClass : StartItemClasses)
			{
				if (ItemClass)
				{
					AGunnerSlotItem* NewSlotItem = GetWorld()->SpawnActor<AGunnerSlotItem>(ItemClass);
					check(NewSlotItem);
					AuthAddItemToSlot(NewSlotItem);
				}
			}
		}

		if (ActionComponent->IsAgentLocallyControlled())
		{
			TArray<AGunnerSlotItem*> OldItems;
			OldItems.SetNum(SlotItems.Num());
			OnRep_SlotItems(OldItems);
		}

		UNexusProperty* SlotIndexProperty = ActionComponent->GetProperty(TAG_Property_SlotIndex);
		check(SlotIndexProperty);
		SlotIndexProperty->OnChangedDelegate.AddDynamic(this, &ThisClass::OnSlotIndexChanged);
		OnSlotIndexChanged(SlotIndexProperty->GetStaticValue(), SlotIndexProperty->GetStaticValue());
	}));
}

void UGunnerSlotManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetOwner()))
	{
		ActionComponent->RemoveSetupCompletedDelegate(this);
	}
}

void UGunnerSlotManagerComponent::AuthAddItemToSlot(AGunnerSlotItem* Item)
{
	check(Item);
	AActor* ActorOwner = GetOwner();
	check(ActorOwner);

	check(ActorOwner->HasAuthority());

	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetOwner());
	check(ActionComponent);
	Item->SetOwner(ActionComponent->GetAgentActor());

	SlotItems[static_cast<int>(Item->GetSlotType())] = Item;
	GR_VLOG_SUB(ActorOwner, LogGunner, Log, TEXT("아이템 [%s]가 슬롯 [%s]에 추가되었습니다"), *Item->GetName(), *UEnum::GetValueAsString(Item->GetSlotType()));
}

void UGunnerSlotManagerComponent::AuthRemoveItemFromSlot(AGunnerSlotItem* Item, bool bDestroyItem)
{
	check(Item);
	AActor* ActorOwner = GetOwner();
	check(ActorOwner);
	check(ActorOwner->HasAuthority());
	SlotItems.Remove(Item);
	if (bDestroyItem)
	{
		Item->Destroy();
	}
	Item->AuthRemovePersistentActivationActions();
	Item->OnRemoved();
}

void UGunnerSlotManagerComponent::OnSlotIndexChanged(float OldValue, float NewValue)
{
	EGunnerSlotType NewActiveSlot = static_cast<EGunnerSlotType>(NewValue);
	EGunnerSlotType OldActiveSlot = static_cast<EGunnerSlotType>(OldValue);

	if (OldActiveSlot != NewActiveSlot)
	{
		if (AGunnerSlotItem* OldItemToDeactivate = OldActiveSlot == EGunnerSlotType::Num ? nullptr : SlotItems[static_cast<int>(OldActiveSlot)])
		{
			DeactivateSlot(OldItemToDeactivate);
		}
	}

	if (AGunnerSlotItem* NewItem = NewActiveSlot == EGunnerSlotType::Num ? nullptr : SlotItems[static_cast<int>(NewActiveSlot)])
	{
		ActivateSlot(NewItem);
	}
}

AGunnerSlotItem* UGunnerSlotManagerComponent::GetCurrentSlotItem() const
{
	EGunnerSlotType CurrentSlotType = static_cast<EGunnerSlotType>(UNexusActionComponent::GetPropertyValueFromActor(GetOwner(), TAG_Property_SlotIndex));
	return CurrentSlotType == EGunnerSlotType::Num ? nullptr : SlotItems[static_cast<int>(CurrentSlotType)];
}

EGunnerSlotType UGunnerSlotManagerComponent::GetCurrentSlotType() const
{
	return static_cast<EGunnerSlotType>(UNexusActionComponent::GetPropertyValueFromActor(GetOwner(), TAG_Property_SlotIndex));
}

void UGunnerSlotManagerComponent::ActivateSlot(AGunnerSlotItem* Item)
{
	check(Item);
	if (Item->IsActivated())
	{
		return;
	}

	AActor* ActorOwner = GetOwner();
	check(ActorOwner);
	if (ActorOwner->HasAuthority())
	{
		Item->AuthAddTransientActivationActions();
	}
	Item->OnActivated();
}

void UGunnerSlotManagerComponent::DeactivateSlot(AGunnerSlotItem* Item)
{
	check(Item);
	if (!Item->IsActivated())
	{
		return;
	}

	AActor* ActorOwner = GetOwner();
	check(ActorOwner);
	if (ActorOwner->HasAuthority())
	{
		Item->AuthRemoveTransientActivationActions();
	}
	Item->OnDeactivated();
}

void UGunnerSlotManagerComponent::OnRep_SlotItems(const TArray<AGunnerSlotItem*>& OldItems)
{
	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetOwner());
	check(ActionComponent);
	if (!ActionComponent->IsSetupCompleted())
	{
		return;
	}

	for (int i = 0; i < static_cast<int>(EGunnerSlotType::Num); ++i)
	{
		if (bool bSlotItemNotChanged = SlotItems[i] == OldItems[i])
		{
			continue;
		}

		if (SlotItems[i])
		{
			ServerAckNewItem(SlotItems[i]);
			if (!GetOwner()->HasAuthority())
			{
				SlotItems[i]->OnAcquired();
				if (GetCurrentSlotType() == static_cast<EGunnerSlotType>(i))
				{
					OnSlotIndexChanged(static_cast<float>(i), static_cast<float>(i));
				}
			}
		}

		if (OldItems[i])
		{
			OldItems[i]->OnRemoved();
		}
	}
}

void UGunnerSlotManagerComponent::ServerAckNewItem_Implementation(AGunnerSlotItem* Item)
{
	check(Item);
	GR_VLOG_SUB(GetOwner(), LogGunner, Log, TEXT("아이템 [%s]가 Ack되었습니다"), *Item->GetName());
	Item->AuthAddPersistentActivationActions();
	Item->OnAcquired();
}

bool UGunnerActionSlotActivation::OnCanTriggerAction() const
{
	bool bCanTrigger = Super::OnCanTriggerAction();
	if (!bCanTrigger)
	{
		return false;
	}

	return GetSlotItem() ? GetSlotItem()->GetSlotType() != GetCurrentSlotType() : false;
}

void UGunnerActionSlotActivation::OnTriggerAction()
{
	Super::OnTriggerAction();
	AActor* AgentActor = GetAgentActor();
	check(AgentActor);
	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(AgentActor);
	check(ActionComponent);
	FNexusSideEffectDef SideEffectDef = UNexusActionComponent::MakeSideEffectDef(this, UNexusSideEffect::StaticClass());
	SideEffectDef.SideEffectInstance->DurationType = ESideEffectDurationType::Instant;

	FNexusPropertyMod Mod;
	Mod.PropertyTag = TAG_Property_SlotIndex;
	Mod.CalculationType = ENexusPropertyCalculationType::FromOutside;
	Mod.Operator = ENexusPropertyOperator::Override;
	Mod.InjectedValueTag = TAG_OperationValue_SlotIndex;

	SideEffectDef.SideEffectInstance->Modifiers.Add(Mod);

	AGunnerSlotItem* SlotItem = GetSlotItem();
	check(SlotItem);
	SideEffectDef.SideEffectInstance->SetInjectedValue(Mod.InjectedValueTag, static_cast<float>(SlotItem->GetSlotType()));

	ActionComponent->TriggerSideEffectByDef(SideEffectDef, this, {}, FNexusPredictionEventSignature::FDelegate::CreateWeakLambda(this, [this, SideEffectDefHandle = SideEffectDef.Handle]()
	{
		GR_VLOG_SUB(GetOwnerActor(), LogGunner, Error, TEXT("슬롯 인덱스 변경 예측 실패"), *SideEffectDefHandle.ToString());
	}));
}

EGunnerSlotType UGunnerActionSlotActivation::GetCurrentSlotType() const
{
	return static_cast<EGunnerSlotType>(UNexusActionComponent::GetPropertyValueFromActor(GetAgentActor(), TAG_Property_SlotIndex));
}
