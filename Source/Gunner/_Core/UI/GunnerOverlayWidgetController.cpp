// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerOverlayWidgetController.h"

#include "Gunner/_Core/ActionSystem/GunnerActionComponent.h"
#include "GameFramework/PlayerState.h"


void UGunnerOverlayWidgetController::InitWidgetController(APlayerState* PlayerState)
{
	UGunnerActionProperty* Property = GetPropertyFromPlayerState(PlayerState, FGameplayTag::RequestGameplayTag("Property.Weapon.MagazineBullet"));
	check(Property);
	Property->OnGunnerActionPropertyValueChangedDelegate.BindUObject(this, &UGunnerOverlayWidgetController::OnMagazineBulletValueChanged);
}

void UGunnerOverlayWidgetController::OnBulletValueChanged(float OldValue, float NewValue)
{
	OnBulletValueChangedDelegate.Broadcast(OldValue, NewValue);
}

void UGunnerOverlayWidgetController::OnMagazineBulletValueChanged(float OldValue, float NewValue)
{
	OnMagazineBulletValueChangedDelegate.Broadcast(OldValue, NewValue);
}

UGunnerActionProperty* UGunnerOverlayWidgetController::GetPropertyFromPlayerState(APlayerState* PlayerState, FGameplayTag Tag)
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

	UGunnerActionProperty* Property = ActionComponent->GetProperty(Tag);
	return Property;
}
