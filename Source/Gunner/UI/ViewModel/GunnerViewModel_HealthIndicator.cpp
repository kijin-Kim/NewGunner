// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerViewModel_HealthIndicator.h"

#include "Action/NexusActionComponent.h"
#include "Action/SubComponent/NexusGameplayTagComponent.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"

void UGunnerViewModel_HealthIndicator::OnCreateViewModel(const UUserWidget* UserWidget)
{
	Super::OnCreateViewModel(UserWidget);
	if (UNexusProperty* HealthProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_Health))
	{
		HealthProperty->OnDirtyDelegate.AddDynamic(this, &UGunnerViewModel_HealthIndicator::OnHealthChanged);
		SetHealth(HealthProperty->GetDynamicValue());
	}

	FOnNexusGameplayTagChangedSignature& OnTagAddedDelegate = ActionComponent->GetOnGameplayTagAddedDelegate();
	if (!OnTagAddedDelegate.IsAlreadyBound(this, &UGunnerViewModel_HealthIndicator::OnTagAdded))
	{
		OnTagAddedDelegate.AddDynamic(this, &UGunnerViewModel_HealthIndicator::OnTagAdded);
	}

	FOnNexusGameplayTagChangedSignature& OnTagRemovedDelegate = ActionComponent->GetOnGameplayTagRemovedDelegate();
	if (!OnTagRemovedDelegate.IsAlreadyBound(this, &UGunnerViewModel_HealthIndicator::OnTagRemoved))
	{
		OnTagRemovedDelegate.AddDynamic(this, &UGunnerViewModel_HealthIndicator::OnTagRemoved);
	}

	if (ActionComponent->HasMatchingGameplayTag(GunnerNativeGameplayTags::TAG_State_Dead))
	{
		SetShouldShowHealthIndicator(false);
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

	FOnNexusGameplayTagChangedSignature& OnTagAddedDelegate = ActionComponent->GetOnGameplayTagAddedDelegate();
	OnTagAddedDelegate.RemoveDynamic(this, &UGunnerViewModel_HealthIndicator::OnTagAdded);
	FOnNexusGameplayTagChangedSignature& OnTagRemovedDelegate = ActionComponent->GetOnGameplayTagRemovedDelegate();
	OnTagRemovedDelegate.RemoveDynamic(this, &UGunnerViewModel_HealthIndicator::OnTagRemoved);
}

void UGunnerViewModel_HealthIndicator::OnHealthChanged(float OldValue, float NewValue)
{
	SetHealth(NewValue);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthIndicatorText);
}

void UGunnerViewModel_HealthIndicator::OnTagAdded(const FGameplayTag& AddedTag)
{
	if (AddedTag == GunnerNativeGameplayTags::TAG_State_Dead)
	{
		SetShouldShowHealthIndicator(false);
	}
}

void UGunnerViewModel_HealthIndicator::OnTagRemoved(const FGameplayTag& RemovedTag)
{
	if (RemovedTag == GunnerNativeGameplayTags::TAG_State_Dead)
	{
		SetShouldShowHealthIndicator(true);
	}
}
