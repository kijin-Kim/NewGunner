// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerSlotManagerViewModel.h"

#include "Action/NexusActionComponent.h"
#include "Gunner/Slot/GunnerSlotManagerComponent.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"


UObject* UGunnerSlotManagerViewModelContextResolver::CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const
{
	return UGunnerSlotManagerViewModel::CreateInstance(GetActionComponent(UserWidget), ExpectedType, UserWidget, View);
}

UGunnerSlotManagerViewModel* UGunnerSlotManagerViewModel::CreateInstance(UNexusActionComponent* ActionComponent, const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View)
{
	UGunnerSlotManagerViewModel* ViewModel = NewObject<UGunnerSlotManagerViewModel>(ActionComponent->GetOwner());
	check(ViewModel);
	ViewModel->Init(ActionComponent);
	return ViewModel;
}

void UGunnerSlotManagerViewModel::Init(UNexusActionComponent* InActionComponent)
{
	ActionComponent = InActionComponent;
	UNexusProperty* BulletProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_Weapon_Bullet);
	check(BulletProperty);
	UNexusProperty* MagazineBulletProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_Weapon_MagazineBullet);
	check(MagazineBulletProperty);
	BulletProperty->OnDirtyDelegate.AddDynamic(this, &UGunnerSlotManagerViewModel::OnBulletCountChanged);
	MagazineBulletProperty->OnDirtyDelegate.AddDynamic(this, &UGunnerSlotManagerViewModel::OnMagazineBulletCountChanged);
}

void UGunnerSlotManagerViewModel::OnBulletCountChanged(float OldValue, float NewValue)
{
	SetBulletCount(NewValue);
}

void UGunnerSlotManagerViewModel::OnMagazineBulletCountChanged(float OldValue, float NewValue)
{
	SetMagazineBulletCount(NewValue);
}
