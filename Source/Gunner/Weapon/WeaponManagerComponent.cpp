// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponManagerComponent.h"

#include "Gun.h"
#include "Weapon.h"
#include "Gunner/Gunner.h"
#include "..\GunnerCharacter.h"
#include "Net/UnrealNetwork.h"


UWeaponManagerComponent::UWeaponManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	Weapons.SetNum(3);
}

void UWeaponManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UWeaponManagerComponent, Weapons);
	DOREPLIFETIME(UWeaponManagerComponent, CurrentWeapon);
}

void UWeaponManagerComponent::SetupWeaponManager()
{
	GunnerCharacterOwner = GetGunnerCharacterOwnerChecked<AGunnerCharacter>();
	if (GunnerCharacterOwner->HasAuthority())
	{
		Weapons[0] = SpawnWeaponByClass(DefaultPrimaryWeaponClass);
		Weapons[1] = SpawnWeaponByClass(DefaultSecondaryWeaponClass);
		Weapons[2] = SpawnWeaponByClass(DefaultMeleeWeaponClass);
	}
	ChangeCurrentWeapon(1);
}


AWeapon* UWeaponManagerComponent::SpawnWeaponByClass(TSubclassOf<AWeapon> WeaponClass)
{
	if (!WeaponClass)
	{
		return nullptr;
	}

	FActorSpawnParameters ActorSpawnParameters;
	ActorSpawnParameters.Owner = GetOwner();
	ActorSpawnParameters.Instigator = Cast<APawn>(GetOwner());
	AWeapon* Weapon = GetWorld()->SpawnActor<AWeapon>(WeaponClass, ActorSpawnParameters);
	Weapon->Unequip();
	return Weapon;
}

void UWeaponManagerComponent::OnRep_Weapons()
{
	for (AWeapon* Weapon : Weapons)
	{
		if (!Weapon || Weapon == CurrentWeapon)
		{
			continue;
		}
		Weapon->Unequip();
	}
}


void UWeaponManagerComponent::OnRep_CurrentWeapon(AWeapon* LastWeapon)
{
	if (LastWeapon)
	{
		LastWeapon->Unequip();
	}
	CurrentWeapon->Equip();
}

void UWeaponManagerComponent::ChangeCurrentWeapon(uint32 WeaponIndex)
{
	if (!CanChangeCurrentWeapon(WeaponIndex))
	{
		return;
	}

	if (GunnerCharacterOwner->IsLocallyControlled() && !GunnerCharacterOwner->HasAuthority()) // ROLE_Autonomous
	{
		LocalChangeCurrentWeapon(WeaponIndex);
		ServerChangeCurrentWeapon(WeaponIndex);
	}
	else
	{
		LocalChangeCurrentWeapon(WeaponIndex);
	}
}

bool UWeaponManagerComponent::CanChangeCurrentWeapon(uint32 WeaponIndex) const
{
	if (WeaponIndex >= 3)
	{
		return false;
	}

	AWeapon* NewWeapon = Weapons[WeaponIndex];
	if (!NewWeapon)
	{
		return false;
	}

	return true;
}

void UWeaponManagerComponent::LocalChangeCurrentWeapon(uint32 WeaponIndex)
{
	if (CurrentWeapon)
	{
		CurrentWeapon->Unequip();
	}

	AWeapon* NewWeapon = Weapons[WeaponIndex];
	NewWeapon->Equip();
	CurrentWeapon = NewWeapon;
}

void UWeaponManagerComponent::ServerChangeCurrentWeapon_Implementation(uint32 WeaponIndex)
{
	LocalChangeCurrentWeapon(WeaponIndex);
}
