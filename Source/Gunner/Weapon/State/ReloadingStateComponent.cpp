// Fill out your copyright notice in the Description page of Project Settings.


#include "ReloadingStateComponent.h"

#include "Gunner/Character/GunnerCharacter.h"
#include "Gunner/Core/AnimMontagePlayerComponent.h"
#include "Gunner/Core/AnimMontagePlayerInterface.h"
#include "Gunner/Weapon/Weapon.h"
#include "Gunner/Weapon/WeaponData.h"


class UAnimMontagePlayerComponent;

UReloadingStateComponent::UReloadingStateComponent()
{
}

void UReloadingStateComponent::Enter()
{
	Super::Enter();
	if (GetOwnerRole() < ROLE_Authority)
	{
		LocalReload();
	}
	ServerReload();
}

void UReloadingStateComponent::Exit()
{
	Super::Exit();
	
	AWeapon* Weapon = GetOwner<AWeapon>();
	UAnimMontagePlayerComponent* WeaponAnimMontagePlayer = IAnimMontagePlayerInterface::Execute_GetAnimMontagePlayer(Weapon);
	FWeaponData* WeaponData = Weapon->GetWeaponData();
	WeaponAnimMontagePlayer->StopMontage(WeaponData->TPWeaponReloadMontage, true);
	WeaponAnimMontagePlayer->StopMontage(WeaponData->FPWeaponReloadMontage, false);

	AGunnerCharacter* GunnerCharacter = Weapon->GetGunnerCharacterOwner();
	UAnimMontagePlayerComponent* GunnerCharacterAnimMontagePlayer = IAnimMontagePlayerInterface::Execute_GetAnimMontagePlayer(GunnerCharacter);
	if(FOnMontageEnded* OnMontageEnded = GunnerCharacterAnimMontagePlayer->GetMontageEndedDelegate(WeaponData->TPCharacterReloadMontage, true))
	{
		OnMontageEnded->Unbind();
	}
	
	GunnerCharacterAnimMontagePlayer->StopMontage(WeaponData->TPCharacterReloadMontage, true);
	GunnerCharacterAnimMontagePlayer->StopMontage(WeaponData->FPCharacterReloadMontage, false);
}

void UReloadingStateComponent::LocalReload()
{
	AWeapon* Weapon = GetOwner<AWeapon>();
	UAnimMontagePlayerComponent* WeaponAnimMontagePlayer = IAnimMontagePlayerInterface::Execute_GetAnimMontagePlayer(Weapon);
	FWeaponData* WeaponData = Weapon->GetWeaponData();
	WeaponAnimMontagePlayer->PlayMontage(WeaponData->TPWeaponReloadMontage, true);
	WeaponAnimMontagePlayer->PlayMontage(WeaponData->FPWeaponReloadMontage, false);

	AGunnerCharacter* GunnerCharacter = Weapon->GetGunnerCharacterOwner();
	UAnimMontagePlayerComponent* GunnerCharacterAnimMontagePlayer = IAnimMontagePlayerInterface::Execute_GetAnimMontagePlayer(GunnerCharacter);
	GunnerCharacterAnimMontagePlayer->PlayMontage(WeaponData->TPCharacterReloadMontage, true);
	GunnerCharacterAnimMontagePlayer->PlayMontage(WeaponData->FPCharacterReloadMontage, false);

	FOnMontageEnded* OnMontageEnded = GunnerCharacterAnimMontagePlayer->GetMontageEndedDelegate(WeaponData->TPCharacterReloadMontage, true);
	OnMontageEnded->BindLambda([this, Weapon](UAnimMontage* AnimMontage, bool bInterrupted)
	{
		EnterNewState(Weapon->EquippedStateComponentClass);
	});
}

void UReloadingStateComponent::ServerReload_Implementation()
{
	LocalReload();
}
