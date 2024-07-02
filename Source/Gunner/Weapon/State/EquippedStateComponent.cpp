// Fill out your copyright notice in the Description page of Project Settings.


#include "EquippedStateComponent.h"

#include "Gunner/Gunner.h"
#include "Gunner/Weapon/Weapon.h"


// Sets default values for this component's properties
UEquippedStateComponent::UEquippedStateComponent()
{
}

void UEquippedStateComponent::OnPrimaryAction(bool bPressed)
{
	if (bPressed && CanFire())
	{
		AWeapon* Weapon = GetWeapon();
		EnterNewState(Weapon->FiringStateComponentClass);
	}
}

void UEquippedStateComponent::OnReloadAction()
{
	if (CanReload())
	{
		AWeapon* Weapon = GetWeapon();
		EnterNewState(Weapon->ReloadingStateComponentClass);
	}
}

void UEquippedStateComponent::Enter()
{
	Super::Enter();
	AWeapon* Weapon = GetWeapon();
	Weapon->OnPrimaryActionDelegate.AddUniqueDynamic(this, &ThisClass::OnPrimaryAction);
	Weapon->OnReloadActionDelegate.AddUniqueDynamic(this, &ThisClass::OnReloadAction);
}

void UEquippedStateComponent::Exit()
{
	Super::Exit();
	AWeapon* Weapon = GetWeapon();
	Weapon->OnPrimaryActionDelegate.RemoveDynamic(this, &ThisClass::OnPrimaryAction);
	Weapon->OnReloadActionDelegate.RemoveDynamic(this, &ThisClass::OnReloadAction);
}

bool UEquippedStateComponent::CanFire() const
{
	AWeapon* Weapon = GetWeapon();
	return Weapon->GetBulletCount() > 0;
}

bool UEquippedStateComponent::CanReload() const
{
	AWeapon* Weapon = GetWeapon();
	return Weapon->GetMagazineBulletCount() > 0 && Weapon->GetMaxBulletCount() > Weapon->GetBulletCount();
}
