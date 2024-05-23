// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponFireComponent.h"

#include "Weapon.h"
#include "Gunner/GunnerCharacter.h"
#include "Kismet/GameplayStatics.h"


UWeaponFireComponent::UWeaponFireComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}

void UWeaponFireComponent::InitializeComponent()
{
	Super::InitializeComponent();
	AWeapon* Weapon = GetOwner<AWeapon>();
	check(Weapon);
	Weapon->OnPrimaryActionDelegate.AddUniqueDynamic(this, &ThisClass::OnPrimaryAction);
}

void UWeaponFireComponent::OnPrimaryAction(bool bPressed)
{
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
	}
	
	if (bPressed && OnWeaponFiredDelegate.IsBound())
	{
		OnWeaponFiredDelegate.Broadcast();
	}
}
