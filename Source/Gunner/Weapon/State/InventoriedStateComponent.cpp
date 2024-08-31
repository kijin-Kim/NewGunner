// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoriedStateComponent.h"

#include "Gunner/Gunner.h"
#include "Gunner/Character/GunnerCharacter.h"
#include "Gunner/Weapon/Weapon.h"


// Sets default values for this component's properties
UInventoriedStateComponent::UInventoriedStateComponent()
{

}

void UInventoriedStateComponent::Enter()
{
	Super::Enter();
	AWeapon* Weapon = GetWeapon();
	IAnimMontagePlayerInterface::Execute_GetFirstPersonMeshComponent(Weapon)->SetVisibility(false);
	IAnimMontagePlayerInterface::Execute_GetThirdPersonMeshComponent(Weapon)->SetVisibility(false);
	Weapon->GetFirstPersonMagainzeMeshComponent()->SetVisibility(false);
	Weapon->GetThirdPersonMagainzeMeshComponent()->SetVisibility(false);
	

	
	if (AGunnerCharacter* GunnerCharacterOwner = Weapon->GetGunnerCharacterOwner())
	{

		USkeletalMeshComponent* GunnerFPMesh = IAnimMontagePlayerInterface::Execute_GetFirstPersonMeshComponent(GunnerCharacterOwner);
		USkeletalMeshComponent* GunnerTPMesh = IAnimMontagePlayerInterface::Execute_GetThirdPersonMeshComponent(GunnerCharacterOwner);

		USkeletalMeshComponent* WeaponFPMesh = IAnimMontagePlayerInterface::Execute_GetFirstPersonMeshComponent(Weapon);
		USkeletalMeshComponent* WeaponTPMesh = IAnimMontagePlayerInterface::Execute_GetThirdPersonMeshComponent(Weapon);
		
		FName FPWeaponSocketName = TEXT("WeaponPoint");
		FName TPWeaponSocketName = TEXT("WeaponPoint");

		Weapon->AttachToComponent(GunnerTPMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("CameraSocket"));
		WeaponFPMesh->AttachToComponent(GunnerFPMesh, FAttachmentTransformRules::KeepRelativeTransform, FPWeaponSocketName);
		WeaponTPMesh->AttachToComponent(GunnerTPMesh, FAttachmentTransformRules::KeepRelativeTransform, TPWeaponSocketName);
	}
	
}
