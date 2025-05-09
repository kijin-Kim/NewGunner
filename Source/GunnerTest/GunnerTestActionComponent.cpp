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
		AuthAddProperty(GunnerNativeGameplayTags::TAG_Property_Bullet);
		AuthAddProperty(GunnerNativeGameplayTags::TAG_Property_MagazineBullet);
		AuthAddProperty(GunnerNativeGameplayTags::TAG_Property_MaxBulletPerMagazine);
		AuthAddProperty(GunnerNativeGameplayTags::TAG_Property_SlotIndex);
	}
}
