// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerOverlayWidgetController.h"

#include "Gunner/_Core/ActionSystem/GunnerActionComponent.h"
#include "GameFramework/PlayerState.h"


void UGunnerOverlayWidgetController::InitWidgetController(APlayerState* PlayerState)
{
	FGunnerActionProperty* PropertyPtr = GetPropertyFromPlayerState(PlayerState, FGameplayTag::RequestGameplayTag("Property.Weapon.MagazineBullet"));
	check(PropertyPtr);
	PropertyPtr->OnGunnerActionPropertyValueChangedDelegate.BindUObject(this, &UGunnerOverlayWidgetController::OnMagazineBulletValueChanged);
}

void UGunnerOverlayWidgetController::OnBulletValueChanged(float OldValue, float NewValue)
{
	OnBulletValueChangedDelegate.Broadcast(OldValue, NewValue);
}

void UGunnerOverlayWidgetController::OnMagazineBulletValueChanged(float OldValue, float NewValue)
{
	OnMagazineBulletValueChangedDelegate.Broadcast(OldValue, NewValue);
}

FGunnerActionProperty* UGunnerOverlayWidgetController::GetPropertyFromPlayerState(APlayerState* PlayerState, FGameplayTag Tag)
{
	if (!PlayerState || !Tag.IsValid())
	{
		return nullptr;
	}

	UGunnerActionComponent* ActionComponent = UGunnerActionComponent::GetActionComponentFromActor(PlayerState);
	if (!ActionComponent)
	{
		return nullptr;
	}

	FGunnerActionProperty* Property = ActionComponent->GetProperty(Tag);
	return Property;
}
