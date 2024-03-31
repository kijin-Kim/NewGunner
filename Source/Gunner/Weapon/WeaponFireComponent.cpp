// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponFireComponent.h"

#include "EnhancedInputComponent.h"
#include "Weapon.h"
#include "Gunner/Gunner.h"
#include "Gunner/GunnerCharacter.h"


// Sets default values for this component's properties
UWeaponFireComponent::UWeaponFireComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
}

void UWeaponFireComponent::InitializeComponent()
{
	Super::InitializeComponent();
	AWeapon* Weapon = GetOwner<AWeapon>();
	Weapon->OnWeaponEquipDelegate.AddDynamic(this, &ThisClass::OnWeaponEquip);
	Weapon->OnWeaponUnequipDelegate.AddDynamic(this, &ThisClass::OnWeaponUnequip);
}

void UWeaponFireComponent::DestroyComponent(bool bPromoteChildren)
{
	Super::DestroyComponent(bPromoteChildren);
	AWeapon* Weapon = GetOwner<AWeapon>();
	Weapon->OnWeaponEquipDelegate.RemoveDynamic(this, &ThisClass::OnWeaponEquip);
	Weapon->OnWeaponUnequipDelegate.RemoveDynamic(this, &ThisClass::OnWeaponUnequip);
}

void UWeaponFireComponent::OnWeaponEquip()
{
	AWeapon* Weapon = GetOwner<AWeapon>();
	Weapon->GetGunnerCharacterOwner()->OnFirePressedDelegate.AddDynamic(this, &ThisClass::Fire);
}

void UWeaponFireComponent::OnWeaponUnequip()
{
	AWeapon* Weapon = GetOwner<AWeapon>();
	Weapon->GetGunnerCharacterOwner()->OnFirePressedDelegate.RemoveDynamic(this, &ThisClass::Fire);
}

void UWeaponFireComponent::Fire()
{
	GR_LOG(LogGunner, Warning, TEXT("Fire"));
}
