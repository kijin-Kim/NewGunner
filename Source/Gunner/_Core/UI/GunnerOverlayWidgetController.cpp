// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerOverlayWidgetController.h"
#include "Action/NexusActionComponent.h"
#include "GameFramework/PlayerState.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"


void UGunnerOverlayWidgetController::InitWidgetController(APlayerState* PlayerState)
{
	check(PlayerState);
	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(PlayerState);
	check(ActionComponent);

	UNexusProperty* BulletProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_Weapon_Bullet);
	check(BulletProperty);
	BulletProperty->OnDirtyDelegate.AddDynamic(this, &UGunnerOverlayWidgetController::OnBulletValueChanged);
	OnBulletValueChanged(BulletProperty->GetStaticValue(), BulletProperty->GetDynamicValue());

	UNexusProperty* MagazineBulletProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_Weapon_MagazineBullet);
	check(MagazineBulletProperty);
	MagazineBulletProperty->OnDirtyDelegate.AddDynamic(this, &UGunnerOverlayWidgetController::OnMagazineBulletValueChanged);
	OnMagazineBulletValueChanged(MagazineBulletProperty->GetStaticValue(), MagazineBulletProperty->GetDynamicValue());
}

void UGunnerOverlayWidgetController::OnBulletValueChanged(float OldValue, float NewValue)
{
	if (OldValue != NewValue)
	{
		OnBulletValueChangedDelegate.Broadcast(OldValue, NewValue);
	}
}

void UGunnerOverlayWidgetController::OnMagazineBulletValueChanged(float OldValue, float NewValue)
{
	if (OldValue != NewValue)
	{
		OnMagazineBulletValueChangedDelegate.Broadcast(OldValue, NewValue);
	}
}
