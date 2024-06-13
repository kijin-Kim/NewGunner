// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponFireComponent.h"
#include "Weapon.h"
#include "WeaponData.h"
#include "Gunner/GunnerCharacter.h"
#include "Gunner/Core/AnimMontagePlayerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


UWeaponFireComponent::UWeaponFireComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
	SetIsReplicatedByDefault(true);
}

void UWeaponFireComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UWeaponFireComponent, BulletCount);
	DOREPLIFETIME(UWeaponFireComponent, MagazineBulletCount);
}

void UWeaponFireComponent::InitializeComponent()
{
	Super::InitializeComponent();
	AWeapon* Weapon = GetOwner<AWeapon>();
	check(Weapon);
	Weapon->OnReloadActionDelegate.AddUniqueDynamic(this, &ThisClass::OnReload);
	Weapon->OnPrimaryActionDelegate.AddUniqueDynamic(this, &ThisClass::OnPrimaryAction);
}

void UWeaponFireComponent::OnRegister()
{
	Super::OnRegister();
	BulletCount = MaxBulletCount;
	MagazineBulletCount = MaxMagazineBulletCount;
}

void UWeaponFireComponent::OnPrimaryAction(bool bPressed)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		LocalFire(bPressed);
	}
	ServerFire(bPressed);
}

void UWeaponFireComponent::ServerFire_Implementation(bool bPressed)
{
	if (bPressed)
	{
		AWeapon* Weapon = GetOwner<AWeapon>();
		AGunnerCharacter* GunnerCharacterOwner = Weapon->GetGunnerCharacterOwner();
		APlayerCameraManager* PlayerCameraManager = Cast<APlayerController>(GunnerCharacterOwner->GetController())->PlayerCameraManager;

		FVector CameraLocation = PlayerCameraManager->GetCameraLocation();
		FVector Forward = PlayerCameraManager->GetCameraRotation().Vector();
		TArray<FHitResult> HitResults;
		FCollisionResponseParams ResponseParams;
		ResponseParams.CollisionResponse.SetAllChannels(ECR_Overlap);
		GetWorld()->LineTraceMultiByChannel(HitResults, CameraLocation, CameraLocation + Forward * 100000.0f, ECC_Visibility, FCollisionQueryParams::DefaultQueryParam, ResponseParams);
		TArray<AActor*> AlreadyHitRegisteredActors;
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor == GunnerCharacterOwner || AlreadyHitRegisteredActors.Find(HitActor) != INDEX_NONE)
			{
				continue;
			}
			AlreadyHitRegisteredActors.Add(HitActor);
			UGameplayStatics::ApplyPointDamage(HitActor, 10.0f, Forward, Hit, GunnerCharacterOwner->GetController(), Weapon, UDamageType::StaticClass());
			DrawDebugSphere(GetWorld(), Hit.Location, 10.0f, 16, FColor::Red, false, 3.0f);
		}
		OnWeaponFired();
		LocalFire(bPressed);
	}
}

void UWeaponFireComponent::LocalFire(bool bPressed)
{
	AWeapon* Weapon = GetOwner<AWeapon>();
	UAnimMontagePlayerComponent* WeaponAnimMontagePlayer = IAnimMontagePlayerInterface::Execute_GetAnimMontagePlayer(Weapon);
	WeaponAnimMontagePlayer->PlayMontage(Weapon->GetWeaponData()->TPWeaponFireMontage, true);
	WeaponAnimMontagePlayer->PlayMontage(Weapon->GetWeaponData()->FPWeaponFireMontage, false);
	
	AGunnerCharacter* GunnerCharacter = Weapon->GetGunnerCharacterOwner();
	UAnimMontagePlayerComponent* GunnerCharacterAnimMontagePlayer = IAnimMontagePlayerInterface::Execute_GetAnimMontagePlayer(GunnerCharacter);
	GunnerCharacterAnimMontagePlayer->PlayMontage(Weapon->GetWeaponData()->TPCharacterFireMontage, true);
	GunnerCharacterAnimMontagePlayer->PlayMontage(Weapon->GetWeaponData()->FPCharacterFireMontage, false);
}

void UWeaponFireComponent::OnWeaponFired()
{
	BulletCount--;
	BulletCount = FMath::Clamp(BulletCount, 0, MaxBulletCount);
	MagazineBulletCount = FMath::Max(MagazineBulletCount, 0);
	if (OnWeaponBulletCountChangedDelegate.IsBound())
	{
		OnWeaponBulletCountChangedDelegate.Broadcast(BulletCount, MagazineBulletCount);
	}


	if (BulletCount == 0) // Auto Reload
	{
		OnReload();
	}
}

void UWeaponFireComponent::OnReload()
{
	ServerReload();
}

bool UWeaponFireComponent::CanReload() const
{
	return MagazineBulletCount > 0;
}

void UWeaponFireComponent::ServerReload_Implementation()
{
	if (!CanReload())
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

void UWeaponFireComponent::OnRep_Bullet()
{
	if (OnWeaponBulletCountChangedDelegate.IsBound())
	{
		OnWeaponBulletCountChangedDelegate.Broadcast(BulletCount, MagazineBulletCount);
	}
}
