// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

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
}

void AWeapon::Unequip()
{
	FirstPersonMeshComponent->SetVisibility(false);
	ThirdPersonMeshComponent->SetVisibility(false);
}

void AWeapon::AttachMeshes()
{
	if (AGunnerCharacterBase* GunnerCharacterOwner = GetGunnerCharacterOwner())
	{
		FirstPersonMeshComponent->AttachToComponent(GunnerCharacterOwner->GetFirstPersonMeshComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale, FirstPersonWeaponSocketName);
		ThirdPersonMeshComponent->AttachToComponent(GunnerCharacterOwner->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, ThirdPersonWeaponSocketName);
	}
}

AGunnerCharacterBase* AWeapon::GetGunnerCharacterOwner() const
{
	return PrivateGunnerCharacterOwner = PrivateGunnerCharacterOwner ? PrivateGunnerCharacterOwner.Get() : Cast<AGunnerCharacterBase>(GetOwner());
}
