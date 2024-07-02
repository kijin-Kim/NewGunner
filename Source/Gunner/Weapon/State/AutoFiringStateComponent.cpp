// Fill out your copyright notice in the Description page of Project Settings.


#include "AutoFiringStateComponent.h"

#include "Gunner/Weapon/Weapon.h"


UAutoFiringStateComponent::UAutoFiringStateComponent()
{
	SetIsReplicatedByDefault(true);
}

void UAutoFiringStateComponent::OnPrimaryAction(bool bPressed)
{
	if(!bPressed)
	{
		AWeapon* Weapon = GetWeapon();
		EnterNewState(Weapon->EquippedStateComponentClass);
	}
}

void UAutoFiringStateComponent::Enter()
{
	Super::Enter();
	AWeapon* Weapon = GetWeapon();
	Weapon->OnPrimaryActionDelegate.AddUniqueDynamic(this, &ThisClass::OnPrimaryAction);
}

void UAutoFiringStateComponent::Exit()
{
	Super::Exit();
	GetWorld()->GetTimerManager().ClearTimer(FireFinishTimer);
	AWeapon* Weapon = GetWeapon();
	Weapon->OnPrimaryActionDelegate.RemoveDynamic(this, &ThisClass::OnPrimaryAction);
}

void UAutoFiringStateComponent::LocalFire()
{
	PlayFireMontage();
	
	AWeapon* Weapon = GetWeapon();
	GetWorld()->GetTimerManager().SetTimer(FireFinishTimer, [this, Weapon]()
	{
		if(CanFire())
		{
			LocalFire();
		}
		else
		{
			EnterNewState(Weapon->EquippedStateComponentClass);
		}
		
	}, Weapon->GetFiringDelay(), false);

	
	Weapon->SetBulletCount(Weapon->GetBulletCount() - 1);
}

bool UAutoFiringStateComponent::CanFire() const
{
	AWeapon* Weapon = GetWeapon();
	return Weapon->GetBulletCount() > 0;
}