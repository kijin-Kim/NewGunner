// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerViewModel_HealthIndicator.h"

#include "Action/NexusActionComponent.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"

void UGunnerViewModel_HealthIndicator::OnCreateViewModel(const UUserWidget* UserWidget)
{
	Super::OnCreateViewModel(UserWidget);
	if (UNexusProperty* HealthProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_Health))
	{
		HealthProperty->OnDirtyDelegate.AddDynamic(this, &UGunnerViewModel_HealthIndicator::OnHealthChanged);
		SetHealth(HealthProperty->GetDynamicValue());
	}
}

void UGunnerViewModel_HealthIndicator::OnDestroyViewModel(const UObject* Object, const UMVVMView* View) const
{
	Super::OnDestroyViewModel(Object, View);
	if (!ActionComponent)
	{
		return;
	}

	if (UNexusProperty* HealthProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_Health))
	{
		HealthProperty->OnDirtyDelegate.RemoveDynamic(this, &UGunnerViewModel_HealthIndicator::OnHealthChanged);
	}

	
}

void UGunnerViewModel_HealthIndicator::OnHealthChanged(float OldValue, float NewValue)
{
	SetHealth(NewValue);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthIndicatorText);
}


