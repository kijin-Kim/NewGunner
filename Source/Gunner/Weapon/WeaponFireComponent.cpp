// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponFireComponent.h"

#include "Weapon.h"


UWeaponFireComponent::UWeaponFireComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}

void UWeaponFireComponent::InitializeComponent()
{
	Super::InitializeComponent();
	AWeapon* Weapon = GetOwner<AWeapon>();
	check(Weapon);
	Weapon->OnPrimaryActionDelegate.AddUniqueDynamic(this, &ThisClass::OnPrimaryAction);
}

void UWeaponFireComponent::OnPrimaryAction(bool bPressed)
{
	if (bPressed && OnWeaponFiredDelegate.IsBound())
	{
		OnWeaponFiredDelegate.Broadcast();
	}
}
