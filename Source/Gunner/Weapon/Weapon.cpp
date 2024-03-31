// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "WeaponData.h"
#include "Gunner/GunnerCharacter.h"


AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	DefaultSceneRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	SetRootComponent(DefaultSceneRootComponent);
	FirstPersonMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMeshComponent->SetupAttachment(GetRootComponent());
	ThirdPersonMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ThirdPersonMesh"));
	ThirdPersonMeshComponent->SetupAttachment(GetRootComponent());

	FirstPersonMeshComponent->bOnlyOwnerSee = true;
	ThirdPersonMeshComponent->bOwnerNoSee = true;
}

void AWeapon::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);
	AttachMeshes();
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	AttachMeshes();
}

void AWeapon::Equip()
{
	FirstPersonMeshComponent->SetVisibility(true);
	ThirdPersonMeshComponent->SetVisibility(true);
	AGunnerCharacter* GunnerCharacterOwner = GetGunnerCharacterOwner();

	if (GunnerCharacterOwner && WeaponData)
	{
		USkeletalMeshComponent* GunnerFirstPersonMeshComponent = GunnerCharacterOwner->GetFirstPersonMeshComponent();
		GunnerFirstPersonMeshComponent->SetAnimInstanceClass(WeaponData->FPCharacterAnimInstance);
		if (UAnimInstance* AnimInstance = GunnerFirstPersonMeshComponent->GetAnimInstance())
		{
			if (WeaponData->FPCharacterEquipMontage)
			{
				AnimInstance->Montage_Play(WeaponData->FPCharacterEquipMontage);
			}
		}

		USkeletalMeshComponent* GunnerThirdPersonMeshComponent = GunnerCharacterOwner->GetMesh();
		GunnerThirdPersonMeshComponent->SetAnimInstanceClass(WeaponData->TPCharacterAnimInstance);
		if (UAnimInstance* AnimInstance = GunnerThirdPersonMeshComponent->GetAnimInstance())
		{
			if (WeaponData->TPCharacterEquipMontage)
			{
				AnimInstance->Montage_Play(WeaponData->TPCharacterEquipMontage);
			}
		}
	}


	if (WeaponData->FPWeaponEquipMontage)
	{
		if (UAnimInstance* AnimInstance = FirstPersonMeshComponent->GetAnimInstance())
		{
			AnimInstance->Montage_Play(WeaponData->FPWeaponEquipMontage);
		}
	}
	if (WeaponData->TPWeaponEquipMontage)
	{
		if (UAnimInstance* AnimInstance = ThirdPersonMeshComponent->GetAnimInstance())
		{
			AnimInstance->Montage_Play(WeaponData->TPWeaponEquipMontage);
		}
	}


	if (OnWeaponEquipDelegate.IsBound())
	{
		OnWeaponEquipDelegate.Broadcast();
	}
}

void AWeapon::Unequip()
{
	if (AGunnerCharacter* GunnerCharacterOwner = GetGunnerCharacterOwner())
	{
		USkeletalMeshComponent* GunnerFirstPersonMeshComponent = GunnerCharacterOwner->GetFirstPersonMeshComponent();
		GunnerFirstPersonMeshComponent->SetAnimInstanceClass(nullptr);
		USkeletalMeshComponent* GunnerThirdPersonMeshComponent = GunnerCharacterOwner->GetMesh();
		GunnerThirdPersonMeshComponent->SetAnimInstanceClass(nullptr);
	}

	FirstPersonMeshComponent->SetVisibility(false);
	ThirdPersonMeshComponent->SetVisibility(false);

	if (OnWeaponUnequipDelegate.IsBound())
	{
		OnWeaponUnequipDelegate.Broadcast();
	}
}

void AWeapon::AttachMeshes()
{
	if (!WeaponData)
	{
		return;
	}

	if (AGunnerCharacter* GunnerCharacterOwner = GetGunnerCharacterOwner())
	{
		FirstPersonMeshComponent->AttachToComponent(GunnerCharacterOwner->GetFirstPersonMeshComponent(), FAttachmentTransformRules::KeepRelativeTransform, WeaponData->FPWeaponSocketName);
		ThirdPersonMeshComponent->AttachToComponent(GunnerCharacterOwner->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, WeaponData->TPWeaponSocketName);
	}
}

AGunnerCharacter* AWeapon::GetGunnerCharacterOwner() const
{
	return PrivateGunnerCharacterOwner = PrivateGunnerCharacterOwner ? PrivateGunnerCharacterOwner.Get() : Cast<AGunnerCharacter>(GetOwner());
}
