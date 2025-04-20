// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerBulletIndicatorViewModel.h"

#include "Action/NexusActionComponent.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"

void UGunnerBulletIndicatorViewModel::OnCreateViewModel(const UUserWidget* UserWidget)
{
	Super::OnCreateViewModel(UserWidget);
	UNexusProperty* BulletProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_Weapon_Bullet);
	check(BulletProperty);
	BulletProperty->OnDirtyDelegate.AddDynamic(this, &UGunnerBulletIndicatorViewModel::OnBulletCountChanged);
	SetBulletCount(BulletProperty->GetDynamicValue());
	
	UNexusProperty* MagazineBulletProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_Weapon_MagazineBullet);
	check(MagazineBulletProperty);
	MagazineBulletProperty->OnDirtyDelegate.AddDynamic(this, &UGunnerBulletIndicatorViewModel::OnMagazineBulletCountChanged);
	SetMagazineBulletCount(MagazineBulletProperty->GetDynamicValue());
}

void UGunnerBulletIndicatorViewModel::OnDestroyViewModel(const UObject* Object, const UMVVMView* View) const
{
	Super::OnDestroyViewModel(Object, View);
	if (!ActionComponent)
	{
		return;
	}

	if (UNexusProperty* BulletProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_Weapon_Bullet))
	{
		BulletProperty->OnDirtyDelegate.RemoveDynamic(this, &UGunnerBulletIndicatorViewModel::OnBulletCountChanged);
	}

	if (UNexusProperty* MagazineBulletProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_Weapon_MagazineBullet))
	{
		MagazineBulletProperty->OnDirtyDelegate.RemoveDynamic(this, &UGunnerBulletIndicatorViewModel::OnMagazineBulletCountChanged);
	}
}

void UGunnerBulletIndicatorViewModel::OnBulletCountChanged(float OldValue, float NewValue)
{
	SetBulletCount(NewValue);
}

void UGunnerBulletIndicatorViewModel::OnMagazineBulletCountChanged(float OldValue, float NewValue)
{
	SetMagazineBulletCount(NewValue);
}
