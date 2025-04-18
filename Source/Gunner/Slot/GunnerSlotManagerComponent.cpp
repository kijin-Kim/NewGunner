// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerSlotManagerComponent.h"

#include "GunnerSlotItemPlaceholder.h"
#include "GunnerSlotManagerInterface.h"
#include "Action/NexusAction.h"
#include "Action/NexusActionComponent.h"
#include "Engine/Canvas.h"
#include "GameFramework/HUD.h"
#include "Gunner/Gunner.h"
#include "Gunner/Action/GunnerActionDropSlotItem.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"
#include "Misc/DataValidation.h"
#include "Net/UnrealNetwork.h"


UGunnerSlotManagerComponent::UGunnerSlotManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	SlotItemInstances.SetNum(static_cast<int>(EGunnerSlotType::Num));
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
		for (AGunnerSlotItem* SlotItem : SlotItemInstances)
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
	DOREPLIFETIME(UGunnerSlotManagerComponent, SlotItemInstances);
}

void UGunnerSlotManagerComponent::BeginPlay()
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

			ensure(!StartItemClasses[static_cast<int>(EGunnerSlotType::Num)]);
			if (!StartItemClasses[static_cast<int>(EGunnerSlotType::Melee)])
			{
				StartItemClasses[static_cast<int>(EGunnerSlotType::Melee)] = AGunnerEquippable_MeleePlaceholder::StaticClass();
			}

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


		UNexusProperty* SlotIndexProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_SlotIndex);
		check(SlotIndexProperty);
		SlotIndexProperty->OnDirtyDelegate.AddDynamic(this, &ThisClass::HandleSlotIndexDirty);
		if (AGunnerSlotItem* CurrentItem = GetCurrentSlotItem())
		{
			OnItemActivated(CurrentItem);
		}
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
	SlotItemInstances[static_cast<int>(Item->GetSlotType())] = Item;
	OnItemAcquired(Item);
	GR_VLOG_SUB(ActorOwner, LogGunner, Log, TEXT("아이템 [%s]가 슬롯 [%s]에 추가되었습니다"), *Item->GetName(), *UEnum::GetValueAsString(Item->GetSlotType()));
}

void UGunnerSlotManagerComponent::AuthRemoveItemFromSlot(AGunnerSlotItem* Item, bool bDestroyItem)
{
	check(Item);
	AActor* ActorOwner = GetOwner();
	check(ActorOwner);
	check(ActorOwner->HasAuthority());
	if (GetCurrentSlotItem() == Item)
	{
		OnItemDeactivated(Item);
	}
	SlotItemInstances[static_cast<int>(Item->GetSlotType())] = nullptr;
	if (bDestroyItem)
	{
		Item->Destroy();
	}

	OnItemRemoved(Item);
}

void UGunnerSlotManagerComponent::HandleSlotIndexDirty(float OldValue, float NewValue)
{
	EGunnerSlotType NewActiveSlot = static_cast<EGunnerSlotType>(NewValue);
	EGunnerSlotType OldActiveSlot = static_cast<EGunnerSlotType>(OldValue);

	if (OldActiveSlot != NewActiveSlot)
	{
		if (AGunnerSlotItem* OldItemToDeactivate = OldActiveSlot == EGunnerSlotType::Num ? nullptr : SlotItemInstances[static_cast<int>(OldActiveSlot)])
		{
			OnItemDeactivated(OldItemToDeactivate);
		}

		if (AGunnerSlotItem* NewItem = (NewActiveSlot == EGunnerSlotType::Num) ? nullptr : SlotItemInstances[static_cast<int>(NewActiveSlot)])
		{
			OnItemActivated(NewItem);
		}
	}
}

AGunnerSlotItem* UGunnerSlotManagerComponent::GetCurrentSlotItem() const
{
	EGunnerSlotType CurrentSlotType = static_cast<EGunnerSlotType>(UNexusActionComponent::GetPropertyValueFromActor(GetOwner(), GunnerNativeGameplayTags::TAG_Property_SlotIndex));
	return CurrentSlotType == EGunnerSlotType::Num ? nullptr : SlotItemInstances[static_cast<int>(CurrentSlotType)];
}

EGunnerSlotType UGunnerSlotManagerComponent::GetCurrentSlotType() const
{
	return static_cast<EGunnerSlotType>(UNexusActionComponent::GetPropertyValueFromActor(GetOwner(), GunnerNativeGameplayTags::TAG_Property_SlotIndex));
}

bool UGunnerSlotManagerComponent::IsSlotEmpty(EGunnerSlotType SlotType) const
{
	return SlotType != EGunnerSlotType::Num && SlotItemInstances[static_cast<int>(SlotType)] == nullptr;
}

AGunnerSlotItem* UGunnerSlotManagerComponent::GetSlotItemByType(EGunnerSlotType SlotType) const
{
	if (SlotType == EGunnerSlotType::Num)
	{
		return nullptr;
	}
	return SlotItemInstances[static_cast<int>(SlotType)];
}

void UGunnerSlotManagerComponent::OnItemAcquired(AGunnerSlotItem* Item)
{
	check(Item);
	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetOwner());
	Item->OnAcquired(ActionComponent->GetAgentActor());
}

void UGunnerSlotManagerComponent::OnItemRemoved(AGunnerSlotItem* Item)
{
	check(Item);
	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetOwner());
	Item->OnRemoved(ActionComponent->GetAgentActor());
}

void UGunnerSlotManagerComponent::OnItemActivated(AGunnerSlotItem* Item)
{
	check(Item);
	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetOwner());
	Item->OnActivated(ActionComponent->GetAgentActor());
}

void UGunnerSlotManagerComponent::OnItemDeactivated(AGunnerSlotItem* Item)
{
	check(Item);
	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetOwner());
	Item->OnDeactivated(ActionComponent->GetAgentActor());
}

EGunnerSlotType UGunnerSlotManagerComponent::FindActivableSlotType() const
{
	if (!IsSlotEmpty(EGunnerSlotType::Primary))
	{
		return EGunnerSlotType::Primary;
	}

	if (!IsSlotEmpty(EGunnerSlotType::Secondary))
	{
		return EGunnerSlotType::Secondary;
	}

	return EGunnerSlotType::Melee;
}


void UGunnerSlotManagerComponent::OnRep_SlotItemInstances(const TArray<AGunnerSlotItem*>& OldItems)
{
	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetOwner());
	check(ActionComponent);
	if (!ActionComponent->IsSetupCompleted())
	{
		return;
	}

	for (int i = 0; i < static_cast<int>(EGunnerSlotType::Num); ++i)
	{
		if (SlotItemInstances[i] == OldItems[i])
		{
			continue;
		}

		if (SlotItemInstances[i])
		{
			OnItemAcquired(SlotItemInstances[i]);
			if (SlotItemInstances[i]->GetSlotType() == GetCurrentSlotType())
			{
				OnItemActivated(SlotItemInstances[i]);
			}
		}

		if (OldItems[i])
		{
			if (OldItems[i]->GetSlotType() == GetCurrentSlotType())
			{
				OnItemDeactivated(OldItems[i]);
			}
			OnItemRemoved(OldItems[i]);
		}
	}
}


UGunnerSlotIndexChangeSideEffect::UGunnerSlotIndexChangeSideEffect()
{
	DurationType = ESideEffectDurationType::Instant;

	FNexusPropertyMod Mod;
	Mod.PropertyTag = GunnerNativeGameplayTags::TAG_Property_SlotIndex;
	Mod.CalculationType = ENexusPropertyCalculationType::FromOutside;
	Mod.Operator = ENexusPropertyOperator::Override;
	Mod.InjectedValueTag = GunnerNativeGameplayTags::TAG_OperationValue_SlotIndex;
	Modifiers.Add(Mod);
}

UGunnerActionSlotActivation::UGunnerActionSlotActivation()
{
	bAllowRemoteTrigger = true;
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

	FNexusSideEffectInstanceDef SideEffectInstanceDef{UGunnerSlotIndexChangeSideEffect::StaticClass()};
	AGunnerSlotItem* SlotItem = GetSlotItem();
	check(SlotItem);
	SideEffectInstanceDef.InjectedValues.Add(FNexusInjectedValuePair{GunnerNativeGameplayTags::TAG_OperationValue_SlotIndex, static_cast<float>(SlotItem->GetSlotType())});

	ActionComponent->ApplySideEffectByDef(SideEffectInstanceDef, {}, FNexusPredictionEventSignature::FDelegate::CreateWeakLambda(this, [this]()
	{
		unimplemented(); // 슬롯 인덱스 변경 예측 실패
	}));
}

EGunnerSlotType UGunnerActionSlotActivation::GetCurrentSlotType() const
{
	return static_cast<EGunnerSlotType>(UNexusActionComponent::GetPropertyValueFromActor(GetAgentActor(), GunnerNativeGameplayTags::TAG_Property_SlotIndex));
}
