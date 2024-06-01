// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBulletComponent.h"

#include "Weapon.h"
#include "WeaponFireComponent.h"
#include "Net/UnrealNetwork.h"


UWeaponBulletComponent::UWeaponBulletComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
	SetIsReplicatedByDefault(true);
}

void UWeaponBulletComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UWeaponBulletComponent, BulletCount);
	DOREPLIFETIME(UWeaponBulletComponent, MagazineBulletCount);
}

void UWeaponBulletComponent::InitializeComponent()
{
	Super::InitializeComponent();
	AWeapon* Weapon = GetOwner<AWeapon>();
	check(Weapon);
	Weapon->OnReloadActionDelegate.AddUniqueDynamic(this, &ThisClass::OnReload);
	UWeaponFireComponent* WeaponFireComponent = Weapon->GetComponentByClass<UWeaponFireComponent>();
	if (WeaponFireComponent)
	{
		WeaponFireComponent->OnWeaponFiredDelegate.AddUniqueDynamic(this, &ThisClass::OnWeaponFired);
	}
}

void UWeaponBulletComponent::OnRegister()
{
	Super::OnRegister();
	BulletCount = MaxBulletCount;
	MagazineBulletCount = MaxMagazineBulletCount;
}

void UWeaponBulletComponent::OnWeaponFired()
{
	BulletCount--;
	BulletCount = FMath::Clamp(BulletCount, 0, MaxBulletCount);
	MagazineBulletCount = FMath::Max(MagazineBulletCount, 0);
	if (OnWeaponBulletCountChangedDelegate.IsBound())
	{
		OnWeaponBulletCountChangedDelegate.Broadcast(BulletCount, MagazineBulletCount);
	}
}

void UWeaponBulletComponent::OnReload()
{
	ServerReload();
}

void UWeaponBulletComponent::ServerReload_Implementation()
{
	if (MagazineBulletCount <= 0)
	{
		return;
	}

	int32 BulletCountToReload = FMath::Min(MaxBulletCount - BulletCount, MagazineBulletCount);
	MagazineBulletCount -= BulletCountToReload;
	BulletCount += BulletCountToReload;
	BulletCount = FMath::Clamp(BulletCount, 0, MaxBulletCount);
	MagazineBulletCount = FMath::Max(MagazineBulletCount, 0);
	if (OnWeaponBulletCountChangedDelegate.IsBound())
	{
		OnWeaponBulletCountChangedDelegate.Broadcast(BulletCount, MagazineBulletCount);
	}
}

void UWeaponBulletComponent::OnRep_Bullet()
{
	if (OnWeaponBulletCountChangedDelegate.IsBound())
	{
		OnWeaponBulletCountChangedDelegate.Broadcast(BulletCount, MagazineBulletCount);
	}
}
