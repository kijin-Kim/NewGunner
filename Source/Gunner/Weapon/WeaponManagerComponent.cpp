// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponManagerComponent.h"

#include "EnhancedInputComponent.h"
#include "Weapon.h"
#include "Gunner/Gunner.h"
#include "Net/UnrealNetwork.h"


UWeaponManagerComponent::UWeaponManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
	SetIsReplicatedByDefault(true);
	Weapons.SetNum(3);
}

void UWeaponManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	SetupWeaponManager();
}

void UWeaponManagerComponent::OnReloadButtonPressed()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->OnReloadButtonPressed();
	}
}

void UWeaponManagerComponent::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(PrimaryWeaponEquipAction, ETriggerEvent::Triggered, this, &ThisClass::ChangeCurrentWeapon, static_cast<uint32>(0));
		EnhancedInputComponent->BindAction(SecondaryWeaponEquipAction, ETriggerEvent::Triggered, this, &ThisClass::ChangeCurrentWeapon, static_cast<uint32>(1));
		EnhancedInputComponent->BindAction(MeleeWeaponEquipAction, ETriggerEvent::Triggered, this, &ThisClass::ChangeCurrentWeapon, static_cast<uint32>(2));
		EnhancedInputComponent->BindAction(PrimaryAction, ETriggerEvent::Triggered, this, &ThisClass::OnPrimaryButtonPressed);
		EnhancedInputComponent->BindAction(PrimaryAction, ETriggerEvent::Completed, this, &ThisClass::OnPrimaryButtonReleased);
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &ThisClass::OnReloadButtonPressed);
	}
}

void UWeaponManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UWeaponManagerComponent, Weapons);
	DOREPLIFETIME_CONDITION(UWeaponManagerComponent, CurrentWeapon, COND_SimulatedOnly);
}

void UWeaponManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();
	GunnerCharacterOwner = GetGunnerCharacterOwnerChecked<AGunnerCharacter>();
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

	if (OnWeaponChangedDelegate.IsBound())
	{
		OnWeaponChangedDelegate.Broadcast(LastWeapon, CurrentWeapon);
	}
}

void UWeaponManagerComponent::ChangeCurrentWeapon(uint32 WeaponIndex)
{
	if (!CanChangeCurrentWeapon(WeaponIndex))
	{
		return;
	}

	if (GetOwnerRole() < ROLE_Authority)
	{
		LocalChangeCurrentWeapon(WeaponIndex);
	}

	ServerChangeCurrentWeapon(WeaponIndex);
}

void UWeaponManagerComponent::OnPrimaryButtonPressed()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->OnPrimaryActionButtonPressed();
	}
}

void UWeaponManagerComponent::OnPrimaryButtonReleased()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->OnPrimaryActionButtonReleased();
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

	if (OnWeaponChangedDelegate.IsBound())
	{
		OnWeaponChangedDelegate.Broadcast(CurrentWeapon, NewWeapon);
	}
}

void UWeaponManagerComponent::ServerChangeCurrentWeapon_Implementation(uint32 WeaponIndex)
{
	check(CanChangeCurrentWeapon(WeaponIndex)); // TODO: 롤백 로직
	LocalChangeCurrentWeapon(WeaponIndex);
}
