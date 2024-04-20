// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponFireComponent.h"

#include "Weapon.h"
#include "Gunner/GunnerCharacter.h"


// Sets default values for this component's properties
UWeaponFireComponent::UWeaponFireComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
}

void UWeaponFireComponent::InitializeComponent()
{
	Super::InitializeComponent();
	AWeapon* Weapon = GetOwner<AWeapon>();
	Weapon->OnWeaponEquipDelegate.AddDynamic(this, &ThisClass::OnWeaponEquip);
	Weapon->OnWeaponUnequipDelegate.AddDynamic(this, &ThisClass::OnWeaponUnequip);
}

void UWeaponFireComponent::DestroyComponent(bool bPromoteChildren)
{
	Super::DestroyComponent(bPromoteChildren);
	AWeapon* Weapon = GetOwner<AWeapon>();
	Weapon->OnWeaponEquipDelegate.RemoveDynamic(this, &ThisClass::OnWeaponEquip);
	Weapon->OnWeaponUnequipDelegate.RemoveDynamic(this, &ThisClass::OnWeaponUnequip);
}

void UWeaponFireComponent::OnWeaponEquip()
{
	AWeapon* Weapon = GetOwner<AWeapon>();
	Weapon->GetGunnerCharacterOwner()->OnFirePressedDelegate.AddDynamic(this, &ThisClass::Fire);
}

void UWeaponFireComponent::OnWeaponUnequip()
{
	AWeapon* Weapon = GetOwner<AWeapon>();
	Weapon->GetGunnerCharacterOwner()->OnFirePressedDelegate.RemoveDynamic(this, &ThisClass::Fire);
}

void UWeaponFireComponent::Fire()
{
	TArray<FHitResult> HitResults;
	AWeapon* Weapon = GetOwner<AWeapon>();
	FVector CameraLocation = Cast<APlayerController>(Weapon->GetGunnerCharacterOwner()->GetController())->PlayerCameraManager->GetCameraLocation();
	FVector Forward = Cast<APlayerController>(Weapon->GetGunnerCharacterOwner()->GetController())->PlayerCameraManager->GetCameraRotation().Vector();

	FCollisionResponseParams ResponseParams;
	ResponseParams.CollisionResponse.SetAllChannels(ECR_Overlap);
	GetWorld()->LineTraceMultiByChannel(HitResults, CameraLocation, CameraLocation + Forward * 100000.0f, ECC_Pawn, FCollisionQueryParams::DefaultQueryParam, ResponseParams);

	for(const FHitResult& Hit : HitResults)
	{
		if(Hit.GetActor())
		{
			DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 16.0f, 16, FColor::Red, false, 3.0f);
		}
	}
}
;