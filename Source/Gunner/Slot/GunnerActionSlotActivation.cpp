// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionSlotActivation.h"

#include "GunnerSlotItem.h"
#include "Action/NexusActionComponent.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"


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
	EndAction();
}

EGunnerSlotType UGunnerActionSlotActivation::GetCurrentSlotType() const
{
	return static_cast<EGunnerSlotType>(UNexusActionComponent::GetPropertyValueFromActor(GetAgentActor(), GunnerNativeGameplayTags::TAG_Property_SlotIndex));
}
