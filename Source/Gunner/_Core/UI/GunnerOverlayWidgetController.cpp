// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerOverlayWidgetController.h"
#include "Gunner/_Core/ActionSystem/GunnerActionComponent.h"
#include "GameFramework/PlayerState.h"


void UGunnerOverlayWidgetController::InitWidgetController(APlayerState* PlayerState)
{
	check(PlayerState);
	UGunnerActionComponent* ActionComponent = UGunnerActionComponent::GetActionComponentFromActor(PlayerState);
	check(ActionComponent);

	UGunnerActionProperty* BulletProperty = ActionComponent->GetProperty(FGameplayTag::RequestGameplayTag("Property.Weapon.Bullet"));
	check(BulletProperty);
	BulletProperty->OnGunnerActionPropertyValueChangedDelegate.BindUObject(this, &UGunnerOverlayWidgetController::OnBulletValueChanged);
	OnBulletValueChanged(BulletProperty->GetStaticValue(), BulletProperty->GetDynamicValue());

	UGunnerActionProperty* MagazineBulletProperty = ActionComponent->GetProperty(FGameplayTag::RequestGameplayTag("Property.Weapon.MagazineBullet"));
	check(MagazineBulletProperty);
	MagazineBulletProperty->OnGunnerActionPropertyValueChangedDelegate.BindUObject(this, &UGunnerOverlayWidgetController::OnMagazineBulletValueChanged);
	OnMagazineBulletValueChanged(MagazineBulletProperty->GetStaticValue(), MagazineBulletProperty->GetDynamicValue());
}

void UGunnerOverlayWidgetController::OnBulletValueChanged(float OldValue, float NewValue)
{
	OnBulletValueChangedDelegate.Broadcast(OldValue, NewValue);
}

void UGunnerOverlayWidgetController::OnMagazineBulletValueChanged(float OldValue, float NewValue)
{
	OnMagazineBulletValueChangedDelegate.Broadcast(OldValue, NewValue);
}
