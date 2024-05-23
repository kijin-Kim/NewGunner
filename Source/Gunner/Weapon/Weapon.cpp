// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
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

void AWeapon::OnPrimaryActionButtonPressed()
{
	if (OnPrimaryActionDelegate.IsBound())
	{
		OnPrimaryActionDelegate.Broadcast(true);
	}
}

void AWeapon::OnPrimaryActionButtonReleased()
{
	if (OnPrimaryActionDelegate.IsBound())
	{
		OnPrimaryActionDelegate.Broadcast(false);
	}
}

void AWeapon::OnReloadButtonPressed()
{
	if (OnReloadActionDelegate.IsBound())
	{
		OnReloadActionDelegate.Broadcast();
	}
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

	if (GunnerCharacterOwner)
	{
		USkeletalMeshComponent* GunnerThirdPersonMeshComponent = GunnerCharacterOwner->GetMesh();
		GunnerThirdPersonMeshComponent->SetAnimInstanceClass(TPCharacterAnimInstance);
		if (UAnimInstance* AnimInstance = GunnerThirdPersonMeshComponent->GetAnimInstance())
		{
			if (TPCharacterEquipMontage)
			{
				AnimInstance->Montage_Play(TPCharacterEquipMontage);
			}
		}
	}


	if (TPWeaponEquipMontage)
	{
		if (UAnimInstance* AnimInstance = ThirdPersonMeshComponent->GetAnimInstance())
		{
			AnimInstance->Montage_Play(TPWeaponEquipMontage);
		}
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
}

void AWeapon::AttachMeshes()
{
	if (AGunnerCharacter* GunnerCharacterOwner = GetGunnerCharacterOwner())
	{
		FirstPersonMeshComponent->AttachToComponent(GunnerCharacterOwner->GetFirstPersonMeshComponent(), FAttachmentTransformRules::KeepRelativeTransform, FPWeaponSocketName);
		ThirdPersonMeshComponent->AttachToComponent(GunnerCharacterOwner->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TPWeaponSocketName);
	}
}

AGunnerCharacter* AWeapon::GetGunnerCharacterOwner() const
{
	return PrivateGunnerCharacterOwner = PrivateGunnerCharacterOwner ? PrivateGunnerCharacterOwner.Get() : Cast<AGunnerCharacter>(GetOwner());
}
