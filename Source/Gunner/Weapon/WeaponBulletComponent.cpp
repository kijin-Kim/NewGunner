// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBulletComponent.h"

#include "Weapon.h"
#include "WeaponFireComponent.h"


UWeaponBulletComponent::UWeaponBulletComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
}

void UWeaponBulletComponent::InitializeComponent()
{
	Super::InitializeComponent();
	AWeapon* Weapon = GetOwner<AWeapon>();
	check(Weapon);
	UWeaponFireComponent* WeaponFireComponent = Weapon->GetComponentByClass<UWeaponFireComponent>();
	if(WeaponFireComponent)
	{
		WeaponFireComponent->OnWeaponFiredDelegate.AddUniqueDynamic(this, &ThisClass::OnWeaponFired);
	}
}

void UWeaponBulletComponent::BeginPlay()
{
	Super::BeginPlay();
	BulletCount = MaxBulletCount;
	MagazineCount = MaxMagazineCount;
}

void UWeaponBulletComponent::OnWeaponFired()
{
	BulletCount--;
	BulletCount = FMath::Clamp(BulletCount, 0, MaxBulletCount);
	MagazineCount = FMath::Clamp(MagazineCount, 0, MaxMagazineCount);
	int32 MagazineBulletCount = MaxBulletCount * MagazineCount;
	if(OnWeaponBulletCountChangedDelegate.IsBound())
	{
		OnWeaponBulletCountChangedDelegate.Broadcast(BulletCount, MagazineBulletCount);
	}	
}
