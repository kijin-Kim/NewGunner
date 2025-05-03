// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerViewModel_BulletIndicator.h"

#include "Action/NexusActionComponent.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"

void UGunnerViewModel_BulletIndicator::OnCreateViewModel(const UUserWidget* UserWidget)
{
	Super::OnCreateViewModel(UserWidget);
	UNexusProperty* BulletProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_Bullet);
	check(BulletProperty);
	BulletProperty->OnDirtyDelegate.AddDynamic(this, &UGunnerViewModel_BulletIndicator::OnBulletCountChanged);
	SetBulletCount(BulletProperty->GetDynamicValue());
	
	UNexusProperty* MagazineBulletProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_MagazineBullet);
	check(MagazineBulletProperty);
	MagazineBulletProperty->OnDirtyDelegate.AddDynamic(this, &UGunnerViewModel_BulletIndicator::OnMagazineBulletCountChanged);
	SetMagazineBulletCount(MagazineBulletProperty->GetDynamicValue());
}

void UGunnerViewModel_BulletIndicator::OnDestroyViewModel(const UObject* Object, const UMVVMView* View) const
{
	Super::OnDestroyViewModel(Object, View);
	if (!ActionComponent)
	{
		return;
	}

	if (UNexusProperty* BulletProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_Bullet))
	{
		BulletProperty->OnDirtyDelegate.RemoveDynamic(this, &UGunnerViewModel_BulletIndicator::OnBulletCountChanged);
	}

	if (UNexusProperty* MagazineBulletProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_MagazineBullet))
	{
		MagazineBulletProperty->OnDirtyDelegate.RemoveDynamic(this, &UGunnerViewModel_BulletIndicator::OnMagazineBulletCountChanged);
	}
}

void UGunnerViewModel_BulletIndicator::OnBulletCountChanged(float OldValue, float NewValue)
{
	SetBulletCount(NewValue);
}

void UGunnerViewModel_BulletIndicator::OnMagazineBulletCountChanged(float OldValue, float NewValue)
{
	SetMagazineBulletCount(NewValue);
}
