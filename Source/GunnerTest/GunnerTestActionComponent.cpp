// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerTestActionComponent.h"

#include "Gunner/Action/GunnerAction_DropSlotItem.h"
#include "Gunner/Item/GunnerInventoryManagerComponent.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"


void UGunnerTestActionComponent::OnSetupActionComponent()
{
	Super::OnSetupActionComponent();

	if (GetOwner()->HasAuthority())
	{
		AuthAddProperty(GunnerNativeGameplayTags::TAG_Property_Bullet, 0);
		AuthAddProperty(GunnerNativeGameplayTags::TAG_Property_MagazineBullet, 0);
		AuthAddProperty(GunnerNativeGameplayTags::TAG_Property_MaxBulletPerMagazine, 0);
		AuthAddProperty(GunnerNativeGameplayTags::TAG_Property_SlotIndex, static_cast<float>(EGunnerSlotType::Num));
	}
}
