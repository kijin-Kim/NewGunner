// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerHealthIndicatorViewModel.h"

#include "Action/NexusActionComponent.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"

void UGunnerHealthIndicatorViewModel::OnCreateViewModel(const UUserWidget* UserWidget)
{
	Super::OnCreateViewModel(UserWidget);
	UNexusProperty* HealthProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_Health);
	check(HealthProperty);
	HealthProperty->OnDirtyDelegate.AddDynamic(this, &UGunnerHealthIndicatorViewModel::OnHealthChanged);
	SetHealth(HealthProperty->GetDynamicValue());
}

void UGunnerHealthIndicatorViewModel::OnDestroyViewModel(const UObject* Object, const UMVVMView* View) const
{
	Super::OnDestroyViewModel(Object, View);
	if (!ActionComponent)
	{
		return;
	}

	if (UNexusProperty* HealthProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_Health))
	{
		HealthProperty->OnDirtyDelegate.RemoveDynamic(this, &UGunnerHealthIndicatorViewModel::OnHealthChanged);
	}
}

void UGunnerHealthIndicatorViewModel::OnHealthChanged(float OldValue, float NewValue)
{
	SetHealth(NewValue);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthIndicatorText);
}
