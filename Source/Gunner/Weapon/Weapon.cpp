// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "WeaponData.h"
#include "Gunner/GunnerCharacter.h"
#include "Gunner/Core/AnimMontagePlayerComponent.h"
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
	AnimMontagePlayerComponent = CreateDefaultSubobject<UAnimMontagePlayerComponent>(TEXT("AnimMontagePlayer"));
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
	FWeaponData* WeaponData = GetWeaponData();
	if (UAnimMontagePlayerComponent* GunnerCharacterAnimMontagePlayer = GunnerCharacterOwner ? IAnimMontagePlayerInterface::Execute_GetAnimMontagePlayer(GunnerCharacterOwner) : nullptr)
	{
		GunnerCharacterAnimMontagePlayer->PlayMontage(WeaponData->TPCharacterEquipMontage, true);
		GunnerCharacterAnimMontagePlayer->PlayMontage(WeaponData->FPCharacterEquipMontage, false);
	}

	AnimMontagePlayerComponent->PlayMontage(WeaponData->TPWeaponEquipMontage, true);
	AnimMontagePlayerComponent->PlayMontage(WeaponData->FPWeaponEquipMontage, false);
}

void AWeapon::Unequip()
{
	FirstPersonMeshComponent->SetVisibility(false);
	ThirdPersonMeshComponent->SetVisibility(false);
}

UAnimMontagePlayerComponent* AWeapon::GetAnimMontagePlayer_Implementation()
{
	return AnimMontagePlayerComponent;
}

AGunnerCharacter* AWeapon::GetGunnerCharacterOwner() const
{
	return PrivateGunnerCharacterOwner = PrivateGunnerCharacterOwner ? PrivateGunnerCharacterOwner.Get() : Cast<AGunnerCharacter>(GetOwner());
}

FWeaponData* AWeapon::GetWeaponData() const
{
	if (WeaponDataCache)
	{
		return WeaponDataCache;
	}
	UGunnerGameInstance* GameInstance = GetWorld()->GetGameInstance<UGunnerGameInstance>();
	check(GameInstance);
	UDataTable* WeaponDataTable = GameInstance->GetWeaponDataTable();
	check(WeaponDataTable);
	WeaponDataCache = WeaponDataTable->FindRow<FWeaponData>(WeaponName, TEXT(""));
	check(WeaponDataCache);
	return WeaponDataCache;
}

void AWeapon::AttachMeshes()
{
	if (AGunnerCharacter* GunnerCharacterOwner = GetGunnerCharacterOwner())
	{
		FirstPersonMeshComponent->AttachToComponent(IAnimMontagePlayerInterface::Execute_GetFirstPersonMeshComponent(GunnerCharacterOwner), FAttachmentTransformRules::KeepRelativeTransform, FPWeaponSocketName);
		ThirdPersonMeshComponent->AttachToComponent(IAnimMontagePlayerInterface::Execute_GetThirdPersonMeshComponent(GunnerCharacterOwner), FAttachmentTransformRules::KeepRelativeTransform, TPWeaponSocketName);
	}
}
