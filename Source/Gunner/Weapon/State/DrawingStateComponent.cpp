// Fill out your copyright notice in the Description page of Project Settings.


#include "DrawingStateComponent.h"

#include "EquippedStateComponent.h"
#include "Gunner/GunnerCharacter.h"
#include "Gunner/Core/AnimMontagePlayerComponent.h"
#include "Gunner/Core/AnimMontagePlayerInterface.h"
#include "Gunner/Weapon/Weapon.h"
#include "Gunner/Weapon/WeaponData.h"


UDrawingStateComponent::UDrawingStateComponent()
{
}

void UDrawingStateComponent::Enter()
{
	Super::Enter();
	AWeapon* Weapon = GetWeapon();
	FWeaponData* WeaponData = Weapon->GetWeaponData();
	
	IAnimMontagePlayerInterface::Execute_GetFirstPersonMeshComponent(Weapon)->SetVisibility(true);
	IAnimMontagePlayerInterface::Execute_GetThirdPersonMeshComponent(Weapon)->SetVisibility(true);
	Weapon->GetFirstPersonMagainzeMeshComponent()->SetVisibility(true);
	Weapon->GetThirdPersonMagainzeMeshComponent()->SetVisibility(true);
	
	if (UAnimMontagePlayerComponent* WeaponAnimMontagePlayerComponent = IAnimMontagePlayerInterface::Execute_GetAnimMontagePlayer(Weapon))
	{
		WeaponAnimMontagePlayerComponent->PlayMontage(WeaponData->TPWeaponEquipMontage, true);
		WeaponAnimMontagePlayerComponent->PlayMontage(WeaponData->FPWeaponEquipMontage, false);
	}


	AGunnerCharacter* GunnerCharacterOwner = Weapon->GetGunnerCharacterOwner();
	if (UAnimMontagePlayerComponent* GunnerCharacterAnimMontagePlayer = GunnerCharacterOwner ? IAnimMontagePlayerInterface::Execute_GetAnimMontagePlayer(GunnerCharacterOwner) : nullptr)
	{
		GunnerCharacterAnimMontagePlayer->PlayMontage(WeaponData->TPCharacterEquipMontage, true);
		GunnerCharacterAnimMontagePlayer->PlayMontage(WeaponData->FPCharacterEquipMontage, false);

		if (FOnMontageEnded* OnMontageEnded = GunnerCharacterAnimMontagePlayer->GetMontageEndedDelegate(WeaponData->TPCharacterEquipMontage, true))
		{
			OnMontageEnded->BindUObject(this, &ThisClass::OnWeaponEquipEnded);
		}
	}
}

void UDrawingStateComponent::Exit()
{
	Super::Exit();
	
	AWeapon* Weapon = GetWeapon();
	FWeaponData* WeaponData = Weapon->GetWeaponData();
	
	if (UAnimMontagePlayerComponent* WeaponAnimMontagePlayerComponent = IAnimMontagePlayerInterface::Execute_GetAnimMontagePlayer(Weapon))
	{
		WeaponAnimMontagePlayerComponent->StopMontage(WeaponData->TPWeaponEquipMontage, true);
		WeaponAnimMontagePlayerComponent->StopMontage(WeaponData->FPWeaponEquipMontage, false);
	}


	AGunnerCharacter* GunnerCharacterOwner = Weapon->GetGunnerCharacterOwner();
	if (UAnimMontagePlayerComponent* GunnerCharacterAnimMontagePlayer = GunnerCharacterOwner ? IAnimMontagePlayerInterface::Execute_GetAnimMontagePlayer(GunnerCharacterOwner) : nullptr)
	{
		if (FOnMontageEnded* OnMontageEnded = GunnerCharacterAnimMontagePlayer->GetMontageEndedDelegate(WeaponData->TPCharacterEquipMontage, true))
		{
			OnMontageEnded->Unbind();
		}

		GunnerCharacterAnimMontagePlayer->StopMontage(WeaponData->TPCharacterEquipMontage, true);
		GunnerCharacterAnimMontagePlayer->StopMontage(WeaponData->FPCharacterEquipMontage, false);
	}
}

void UDrawingStateComponent::OnWeaponEquipEnded(UAnimMontage* AnimMontage, bool bInterrupted)
{
	EnterNewState(UEquippedStateComponent::StaticClass());
}
