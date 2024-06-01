// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "WeaponData.h"
#include "Gunner/GunnerCharacter.h"
#include "Gunner/Core/GunnerGameInstance.h"


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

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	UGunnerGameInstance* GameInstance = GetWorld()->GetGameInstance<UGunnerGameInstance>();
	check(GameInstance);
	UDataTable* WeaponDataTable = GameInstance->GetWeaponDataTable();
	check(WeaponDataTable);
	WeaponData = WeaponDataTable->FindRow<FWeaponData>(WeaponName, TEXT(""));
	check(WeaponData);
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
		GunnerThirdPersonMeshComponent->SetAnimInstanceClass(WeaponData->TPCharacterAnimInstance);
		GunnerCharacterOwner->PlayMontage(WeaponData->TPCharacterEquipMontage, true);
	}
	
	PlayMontage(WeaponData->TPWeaponEquipMontage, true);
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

AGunnerCharacter* AWeapon::GetGunnerCharacterOwner() const
{
	return PrivateGunnerCharacterOwner = PrivateGunnerCharacterOwner ? PrivateGunnerCharacterOwner.Get() : Cast<AGunnerCharacter>(GetOwner());
}

void AWeapon::AttachMeshes()
{
	if (AGunnerCharacter* GunnerCharacterOwner = GetGunnerCharacterOwner())
	{
		FirstPersonMeshComponent->AttachToComponent(GunnerCharacterOwner->GetFirstPersonMeshComponent(), FAttachmentTransformRules::KeepRelativeTransform, FPWeaponSocketName);
		ThirdPersonMeshComponent->AttachToComponent(GunnerCharacterOwner->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TPWeaponSocketName);
	}
}


