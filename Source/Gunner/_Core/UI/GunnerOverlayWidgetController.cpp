// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerOverlayWidgetController.h"

#include "Gunner/_Core/ActionSystem/GunnerActionComponent.h"
#include "GameFramework/PlayerState.h"


void UGunnerOverlayWidgetController::InitWidgetController(APlayerState* PlayerState)
{
	if (UGunnerActionComponent* ActionComponent = UGunnerActionComponent::GetActionComponentFromActor(PlayerState))
	{
		ActionComponent->PropertyArray.BindOnGunnerActionPropertyAdded(FGameplayTag::RequestGameplayTag("Property.Weapon.Bullet"), FOnGunnerActionPropertyCountChangedSignature::CreateLambda([this, PlayerState](const FGunnerActionProperty& Property)
		{
			BindOnBulletValueChanged(PlayerState);
		}));

		ActionComponent->PropertyArray.BindOnGunnerActionPropertyRemoved(FGameplayTag::RequestGameplayTag("Property.Weapon.Bullet"), FOnGunnerActionPropertyCountChangedSignature::CreateLambda([this, PlayerState](const FGunnerActionProperty& Property)
		{
			OnBulletValueChanged(0.0f, 0.0f);
		}));
	}

	BindOnBulletValueChanged(PlayerState);


	if (UGunnerActionComponent* ActionComponent = UGunnerActionComponent::GetActionComponentFromActor(PlayerState))
	{
		ActionComponent->PropertyArray.BindOnGunnerActionPropertyAdded(FGameplayTag::RequestGameplayTag("Property.Weapon.MagazineBullet"), FOnGunnerActionPropertyCountChangedSignature::CreateLambda([this, PlayerState](const FGunnerActionProperty& Property)
		{
			if (FGunnerActionProperty* PropertyPtr = GetPropertyFromPlayerState(PlayerState, FGameplayTag::RequestGameplayTag("Property.Weapon.MagazineBullet")))
			{
				PropertyPtr->OnGunnerActionPropertyValueChangedDelegate.BindUObject(this, &UGunnerOverlayWidgetController::OnMagazineBulletValueChanged);
			}
		}));

		ActionComponent->PropertyArray.BindOnGunnerActionPropertyRemoved(FGameplayTag::RequestGameplayTag("Property.Weapon.MagazineBullet"), FOnGunnerActionPropertyCountChangedSignature::CreateLambda([this, PlayerState](const FGunnerActionProperty& Property)
		{
			OnMagazineBulletValueChanged(0.0f, 0.0f);
		}));
	}

	if (FGunnerActionProperty* PropertyPtr = GetPropertyFromPlayerState(PlayerState, FGameplayTag::RequestGameplayTag("Property.Weapon.MagazineBullet")))
	{
		PropertyPtr->OnGunnerActionPropertyValueChangedDelegate.BindUObject(this, &UGunnerOverlayWidgetController::OnMagazineBulletValueChanged);
	}
}

void UGunnerOverlayWidgetController::OnBulletValueChanged(float OldValue, float NewValue)
{
	OnBulletValueChangedDelegate.Broadcast(OldValue, NewValue);
}

void UGunnerOverlayWidgetController::OnMagazineBulletValueChanged(float OldValue, float NewValue)
{
	OnMagazineBulletValueChangedDelegate.Broadcast(OldValue, NewValue);
}

void UGunnerOverlayWidgetController::BindOnBulletValueChanged(APlayerState* PlayerState)
{
	if (FGunnerActionProperty* PropertyPtr = GetPropertyFromPlayerState(PlayerState, FGameplayTag::RequestGameplayTag("Property.Weapon.Bullet")))
	{
		PropertyPtr->OnGunnerActionPropertyValueChangedDelegate.BindUObject(this, &UGunnerOverlayWidgetController::OnBulletValueChanged);
	}
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

	FGunnerActionProperty* Property = ActionComponent->GetProperty2(Tag);
	return Property;
}
