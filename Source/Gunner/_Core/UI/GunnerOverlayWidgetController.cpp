// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerOverlayWidgetController.h"
#include "NexusActionComponent.h"
#include "GameFramework/PlayerState.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"


void UGunnerOverlayWidgetController::InitWidgetController(APlayerState* PlayerState)
{
	check(PlayerState);
	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(PlayerState);
	check(ActionComponent);
	
	UNexusProperty* BulletProperty = ActionComponent->GetProperty(TAG_Property_Weapon_Bullet);
	check(BulletProperty);
	BulletProperty->OnChangedDelegate.AddDynamic(this, &UGunnerOverlayWidgetController::OnBulletValueChanged);
	OnBulletValueChanged(BulletProperty->GetStaticValue(), BulletProperty->GetDynamicValue());

	UNexusProperty* MagazineBulletProperty = ActionComponent->GetProperty(TAG_Property_Weapon_MagazineBullet);
	check(MagazineBulletProperty);
	MagazineBulletProperty->OnChangedDelegate.AddDynamic(this, &UGunnerOverlayWidgetController::OnMagazineBulletValueChanged);
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
