// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "WeaponData.h"
#include "Gunner/Gunner.h"
#include "Gunner/Character/GunnerCharacter.h"
#include "Gunner/Core/AnimMontagePlayerComponent.h"
#include "Gunner/Core/GunnerGameInstance.h"
#include "State/DroppedStateComponent.h"
#include "State/EquippedStateComponent.h"
#include "State/DrawingStateComponent.h"
#include "State/FiringStateComponent.h"
#include "State/InventoriedStateComponent.h"
#include "State/ReloadingStateComponent.h"
#include "State/StateComponent.h"


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
	
	FirstPersonMagazineMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FirstPersonMagazineMesh"));
	FirstPersonMagazineMeshComponent->SetupAttachment(FirstPersonMeshComponent, TEXT("Magazine_Main"));
	FirstPersonMagazineMeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	ThirdPersonMagazineMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ThirdPersonMagazineMesh"));
	ThirdPersonMagazineMeshComponent->SetupAttachment(ThirdPersonMeshComponent, TEXT("Magazine_Main"));
	ThirdPersonMagazineMeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

	FirstPersonMeshComponent->bOnlyOwnerSee = true;
	ThirdPersonMeshComponent->bOwnerNoSee = true;
	FirstPersonMagazineMeshComponent->bOnlyOwnerSee = true;
	ThirdPersonMagazineMeshComponent->bOwnerNoSee = true;
	
	AnimMontagePlayerComponent = CreateDefaultSubobject<UAnimMontagePlayerComponent>(TEXT("AnimMontagePlayer"));


	InventoriedStateComponentClass = UInventoriedStateComponent::StaticClass();
	DrawingStateComponentClass = UDrawingStateComponent::StaticClass();
	EquippedStateComponentClass = UEquippedStateComponent::StaticClass();
	FiringStateComponentClass = UFiringStateComponent::StaticClass();
	ReloadingStateComponentClass = UReloadingStateComponent::StaticClass();
	DroppedStateComponentClass = UDroppedStateComponent::StaticClass();
	
}

void AWeapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	InventoriedStateComponent = NewObject<UInventoriedStateComponent>(this, InventoriedStateComponentClass, FName(TEXT("InventoriedStateComponent")));
	InventoriedStateComponent->RegisterComponent();

	DrawingStateComponent = NewObject<UDrawingStateComponent>(this, DrawingStateComponentClass, FName(TEXT("DrawingStateComponent")));
	DrawingStateComponent->RegisterComponent();
	
	EquippedStateComponent = NewObject<UEquippedStateComponent>(this, EquippedStateComponentClass, FName(TEXT("EquippedStateComponent")));
	EquippedStateComponent->RegisterComponent();
	
	FiringStateComponent = NewObject<UFiringStateComponent>(this, FiringStateComponentClass, FName(TEXT("FiringStateComponent")));
	FiringStateComponent->RegisterComponent();
	
	ReloadingStateComponent = NewObject<UReloadingStateComponent>(this, ReloadingStateComponentClass, FName(TEXT("ReloadingStateComponent")));
	ReloadingStateComponent->RegisterComponent();
	
	DroppedStateComponent = NewObject<UDroppedStateComponent>(this, DroppedStateComponentClass, FName(TEXT("DroppedStateComponent")));
	DroppedStateComponent->RegisterComponent();
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

void AWeapon::Equip()
{
	EnterNewState(DrawingStateComponentClass);
}

void AWeapon::Unequip()
{
	EnterNewState(InventoriedStateComponentClass);
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


void AWeapon::SetBulletCount(int32 InBulletCount)
{
	BulletCount = InBulletCount;
	if (OnWeaponBulletCountChangedDelegate.IsBound())
	{
		OnWeaponBulletCountChangedDelegate.Broadcast(BulletCount, MagazineBulletCount);
	}
}

int32 AWeapon::GetBulletCount() const
{
	return BulletCount;
}

int32 AWeapon::GetMaxMagazineBulletCount() const
{
	return MaxMagazineBulletCount;
}

int32 AWeapon::GetMaxBulletCount() const
{
	return MaxBulletCount;
}

void AWeapon::SetMagazineBulletCount(int32 InMagazineBulletCount)
{
	MagazineBulletCount = InMagazineBulletCount;
	if (OnWeaponBulletCountChangedDelegate.IsBound())
	{
		OnWeaponBulletCountChangedDelegate.Broadcast(BulletCount, MagazineBulletCount);
	}
}

int32 AWeapon::GetMagazineBulletCount() const
{
	return MagazineBulletCount;
}

float AWeapon::GetFiringDelay() const
{
	return FiringDelay;
}

void AWeapon::EnterNewState(TSubclassOf<UStateComponent> NewState)
{
	UStateComponent* NewStateComponent = Cast<UStateComponent>(GetComponentByClass(NewState));
	if (NewStateComponent)
	{
		if (CurrentState)
		{
			CurrentState->Exit();
		}
		CurrentState = NewStateComponent;
		CurrentState->Enter();
	}
}
