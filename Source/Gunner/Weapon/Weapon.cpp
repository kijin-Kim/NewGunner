// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "WeaponData.h"
#include "Gunner/Gunner.h"
#include "Gunner/GunnerCharacterBase.h"


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


	// FP메쉬는 Autonomous Proxy에서만 보임.
	// TP메쉬는 Autonomous Proxy에서만 보이지 않음.
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
	AGunnerCharacterBase* GunnerCharacterOwner = GetGunnerCharacterOwner();

	if (GunnerCharacterOwner && WeaponData && WeaponData->FPCharacterAnimInstanceClass)
	{
		USkeletalMeshComponent* GunnerFirstPersonMeshComponent = GunnerCharacterOwner->GetFirstPersonMeshComponent();
		GunnerFirstPersonMeshComponent->SetAnimInstanceClass(WeaponData->FPCharacterAnimInstanceClass);
		if (UAnimInstance* AnimInstance = GunnerFirstPersonMeshComponent->GetAnimInstance())
		{
			if (WeaponData->FPCharacterEquipMontage)
			{
				AnimInstance->Montage_Play(WeaponData->FPCharacterEquipMontage);
			}
		}

		USkeletalMeshComponent* GunnerThirdPersonMeshComponent = GunnerCharacterOwner->GetMesh();
		GunnerThirdPersonMeshComponent->SetAnimInstanceClass(WeaponData->TPCharacterAnimInstanceClass);
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
}

void AWeapon::Unequip()
{
	FirstPersonMeshComponent->SetVisibility(false);
	ThirdPersonMeshComponent->SetVisibility(false);
	AGunnerCharacterBase* GunnerCharacterOwner = GetGunnerCharacterOwner();
	if (GunnerCharacterOwner)
	{
		GetGunnerCharacterOwner()->GetFirstPersonMeshComponent()->SetAnimInstanceClass(nullptr);
		GetGunnerCharacterOwner()->GetMesh()->SetAnimInstanceClass(nullptr);
	}
}

void AWeapon::AttachMeshes()
{
	if (!WeaponData)
	{
		return;
	}

	if (AGunnerCharacterBase* GunnerCharacterOwner = GetGunnerCharacterOwner())
	{
		FirstPersonMeshComponent->AttachToComponent(GunnerCharacterOwner->GetFirstPersonMeshComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponData->FPWeaponSocketName);
		ThirdPersonMeshComponent->AttachToComponent(GunnerCharacterOwner->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponData->TPWeaponSocketName);
	}
}

AGunnerCharacterBase* AWeapon::GetGunnerCharacterOwner() const
{
	return PrivateGunnerCharacterOwner = PrivateGunnerCharacterOwner ? PrivateGunnerCharacterOwner.Get() : Cast<AGunnerCharacterBase>(GetOwner());
}
