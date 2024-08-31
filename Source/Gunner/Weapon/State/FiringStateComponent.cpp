// Fill out your copyright notice in the Description page of Project Settings.


#include "FiringStateComponent.h"

#include "Gunner/Character/GunnerCharacter.h"
#include "Gunner/Core/AnimMontagePlayerComponent.h"
#include "Gunner/Weapon/Weapon.h"
#include "Gunner/Weapon/WeaponData.h"


UFiringStateComponent::UFiringStateComponent()
{
	SetIsReplicatedByDefault(true);
}

void UFiringStateComponent::Enter()
{
	Super::Enter();
	if (GetOwnerRole() < ROLE_Authority)
	{
		LocalFire();
	}
	ServerFire();

	
}

void UFiringStateComponent::Exit()
{
	Super::Exit();

}

void UFiringStateComponent::PlayFireMontage()
{
	AWeapon* Weapon = GetWeapon();
	FWeaponData* WeaponData = Weapon->GetWeaponData();
	UAnimMontagePlayerComponent* WeaponAnimMontagePlayer = IAnimMontagePlayerInterface::Execute_GetAnimMontagePlayer(Weapon);
	
	WeaponAnimMontagePlayer->PlayMontage(WeaponData->TPWeaponFireMontage, true);
	WeaponAnimMontagePlayer->PlayMontage(WeaponData->FPWeaponFireMontage, false);

	AGunnerCharacter* GunnerCharacter = Weapon->GetGunnerCharacterOwner();
	UAnimMontagePlayerComponent* GunnerCharacterAnimMontagePlayer = IAnimMontagePlayerInterface::Execute_GetAnimMontagePlayer(GunnerCharacter);
	GunnerCharacterAnimMontagePlayer->PlayMontage(WeaponData->TPCharacterFireMontage, true);
	GunnerCharacterAnimMontagePlayer->PlayMontage(WeaponData->FPCharacterFireMontage, false);
}

void UFiringStateComponent::LocalFire()
{
	PlayFireMontage();
	
	AWeapon* Weapon = GetWeapon();
	FTimerHandle FireFinishTimer;
	GetWorld()->GetTimerManager().SetTimer(FireFinishTimer, [this, Weapon]()
	{
		EnterNewState(Weapon->EquippedStateComponentClass);
	}, Weapon->GetFiringDelay(), false);

	
	
	Weapon->SetBulletCount(Weapon->GetBulletCount() - 1);
}

void UFiringStateComponent::ServerFire_Implementation()
{
	LocalFire();
}
