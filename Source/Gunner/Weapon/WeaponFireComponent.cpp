// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponFireComponent.h"

#include "Weapon.h"
#include "Gunner/GunnerCharacter.h"
#include "Kismet/GameplayStatics.h"


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
	AWeapon* Weapon = GetOwner<AWeapon>();
	Weapon->GetGunnerCharacterOwner()->PlayAnimMontage(TPCharacterFireMontage);
	WeaponLineTrace();
	if(GetOwner<AWeapon>()->GetGunnerCharacterOwner()->GetLocalRole() < ROLE_Authority)
	{
		ServerFire();
	}
}

void UWeaponFireComponent::WeaponLineTrace()
{
	TArray<FHitResult> HitResults;
	AWeapon* Weapon = GetOwner<AWeapon>();
	FVector CameraLocation = Cast<APlayerController>(Weapon->GetGunnerCharacterOwner()->GetController())->PlayerCameraManager->GetCameraLocation();
	FVector Forward = Cast<APlayerController>(Weapon->GetGunnerCharacterOwner()->GetController())->PlayerCameraManager->GetCameraRotation().Vector();

	FCollisionResponseParams ResponseParams;
	ResponseParams.CollisionResponse.SetAllChannels(ECR_Overlap);
	GetWorld()->LineTraceMultiByChannel(HitResults, CameraLocation, CameraLocation + Forward * 100000.0f, ECC_Visibility, FCollisionQueryParams::DefaultQueryParam, ResponseParams);

	TArray<AActor*> AlreadyHitRegisteredActors;
	for(const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if(HitActor == Weapon || HitActor == Weapon->GetGunnerCharacterOwner() || AlreadyHitRegisteredActors.Find(HitActor) != INDEX_NONE)
		{
			continue;
		}
		
		
		AlreadyHitRegisteredActors.Add(HitActor);
		UGameplayStatics::ApplyPointDamage(HitActor, 10.0f,  Forward, Hit, Weapon->GetGunnerCharacterOwner()->GetController(), Weapon, UDamageType::StaticClass());
		DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 13.0f, 13, Weapon->GetGunnerCharacterOwner()->GetLocalRole() < ROLE_Authority ? FColor::Blue : FColor::Red, false, 3.0f);
	}
}

void UWeaponFireComponent::ServerFire_Implementation()
{
	WeaponLineTrace();
}
