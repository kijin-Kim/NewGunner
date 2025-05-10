// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerAction_EquipBase.h"

#include "GunnerEquipmentItem.h"
#include "GunnerSlotItem.h"
#include "Action/NexusActionComponent.h"
#include "Blueprint/UserWidget.h"
#include "Gunner/Gunner.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"
#include "Misc/DataValidation.h"


UGunnerSlotIndexChangeSideEffect::UGunnerSlotIndexChangeSideEffect()
{
	DurationType = ENexusSideEffectDurationType::Instant;

	FNexusPropertyMod Mod;
	Mod.PropertyTag = GunnerNativeGameplayTags::TAG_Property_SlotIndex;
	Mod.CalculationType = ENexusPropertyCalculationType::FromOutside;
	Mod.Operator = ENexusPropertyOperator::Override;
	Mod.InjectedValueTag = GunnerNativeGameplayTags::TAG_OperationValue_SlotIndex;
	Modifiers.Add(Mod);
}

EDataValidationResult UGunnerAction_EquipBase::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	for (TSubclassOf<UNexusAction> ActionClass : EquippedActions)
	{
		if (!ActionClass)
		{
			Context.AddError(NSLOCTEXT("Gunner", "InvalidActionClass", "유효하지 않은 액션 클래스 존재"));
			Result = CombineDataValidationResults(Result, EDataValidationResult::Invalid);
		}
	}

	for (TSubclassOf<UUserWidget> WidgetClass : EquippedWidgetClasses)
	{
		if (!WidgetClass)
		{
			Context.AddError(NSLOCTEXT("Gunner", "InvalidWidgetClass", "유효하지 않은 위젯 클래스 존재"));
			Result = CombineDataValidationResults(Result, EDataValidationResult::Invalid);
		}
	}

	return Result;
}

UGunnerAction_EquipBase::UGunnerAction_EquipBase()
{
	bAllowRemoteTrigger = true;
	ActionOwnedTags.AddTag(GunnerNativeGameplayTags::TAG_Action_Equip);
	ActionCancelTags.AddTag(GunnerNativeGameplayTags::TAG_Action_Equip);
}

void UGunnerAction_EquipBase::OnAddAction()
{
	Super::OnAddAction();
	EquipmentItem = GetSourceObject<AGunnerEquipmentItem>();
}

bool UGunnerAction_EquipBase::OnCanTriggerAction() const
{
	bool bCanTrigger = Super::OnCanTriggerAction();
	if (!bCanTrigger)
	{
		return false;
	}

	return EquipmentItem ? EquipmentItem->GetSlotType() != GetCurrentSlotType() : false;
}

void UGunnerAction_EquipBase::OnTriggerAction()
{
	Super::OnTriggerAction();
	AActor* AgentActor = GetAgentActor();
	check(AgentActor);
	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(AgentActor);
	check(ActionComponent);

	if (IsOwnerActorAuthoritative())
	{
		for (TSubclassOf<UNexusAction> ActionClass : EquippedActions)
		{
			if (ActionClass)
			{
				EquippedActionDefHandles.Add(ActionComponent->AuthAddAction(ActionClass, EquipmentItem));
			}
		}
	}

	FNexusSideEffectInstanceDef SideEffectInstanceDef{UGunnerSlotIndexChangeSideEffect::StaticClass()};
	SideEffectInstanceDef.InjectedValues.Add(FNexusInjectedValuePair{GunnerNativeGameplayTags::TAG_OperationValue_SlotIndex, static_cast<float>(EquipmentItem->GetSlotType())});

	ActionComponent->ApplySideEffectByDef(SideEffectInstanceDef, {}, FNexusPredictionEventSignature::FDelegate::CreateWeakLambda(this, [this]()
	{
		unimplemented(); // 슬롯 인덱스 변경 예측 실패
	}));


	if (IsLocallyPlayerControlled())
	{
		for (TSubclassOf<UUserWidget> WidgetClass : EquippedWidgetClasses)
		{
			if (!WidgetClass)
			{
				NX_LOG_SUB(GetAgentActor(), LogGunnerInventory, Warning, TEXT("유효하지 않은 위젯 클래스 존재"));
				continue;
			}

			if (EquippedWidgetMap.Contains(WidgetClass))
			{
				EquippedWidgetMap[WidgetClass]->AddToViewport();
				continue;
			}

			UUserWidget* Widget = CreateWidget<UUserWidget>(Cast<APlayerController>(GetController()), WidgetClass);
			check(Widget);
			Widget->AddToViewport();
			EquippedWidgetMap.Add(WidgetClass, Widget);
		}
	}
}


void UGunnerAction_EquipBase::OnEndAction()
{
	Super::OnEndAction();
	if (IsOwnerActorAuthoritative())
	{
		if (UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(GetOwnerActor()))
		{
			for (const FNexusActionDefHandle& Handle : EquippedActionDefHandles)
			{
				ActionComponent->AuthRemoveAction(Handle);
			}
		}
		
		EquippedActionDefHandles.Empty();
	}

	if (IsLocallyPlayerControlled())
	{
		RemoveEquippedWidgetsFromParent();
	}
}

void UGunnerAction_EquipBase::OnRemoveAction()
{
	Super::OnRemoveAction();
	if (IsLocallyPlayerControlled())
	{
		RemoveEquippedWidgetsFromParent();
	}
	EquippedWidgetMap.Empty();

}

void UGunnerAction_EquipBase::RemoveEquippedWidgetsFromParent()
{
	if (!ensure(IsLocallyControlled()))
	{
		NX_LOG_SUB_FN(GetAgentActor(), LogGunnerInventory, Warning, TEXT("잘못된 함수 호출 (로컬 플레이어가 아님)"));
		return;
	}

	for (const auto& [WidgetClass, Widget] : EquippedWidgetMap)
	{
		if (Widget)
		{
			Widget->RemoveFromParent();
		}
	}
}

EGunnerSlotType UGunnerAction_EquipBase::GetCurrentSlotType() const
{
	return static_cast<EGunnerSlotType>(UNexusActionComponent::GetPropertyValueFromActor(GetAgentActor(), GunnerNativeGameplayTags::TAG_Property_SlotIndex));
}
