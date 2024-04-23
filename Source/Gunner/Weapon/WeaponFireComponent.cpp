// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponFireComponent.h"

#include "Weapon.h"
#include "Gunner/Gunner.h"
#include "Gunner/GunnerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/PhysicsAsset.h"


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
	FlushPersistentDebugLines(GetWorld());
	AWeapon* Weapon = GetOwner<AWeapon>();
	Weapon->GetGunnerCharacterOwner()->PlayAnimMontage(TPCharacterFireMontage);
	WeaponLineTrace();
	if (GetOwner<AWeapon>()->GetGunnerCharacterOwner()->GetLocalRole() < ROLE_Authority)
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
	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor == Weapon || HitActor == Weapon->GetGunnerCharacterOwner() || AlreadyHitRegisteredActors.Find(HitActor) != INDEX_NONE)
		{
			continue;
		}


		AlreadyHitRegisteredActors.Add(HitActor);
		UGameplayStatics::ApplyPointDamage(HitActor, 10.0f, Forward, Hit, Weapon->GetGunnerCharacterOwner()->GetController(), Weapon, UDamageType::StaticClass());
	}


	TArray<AActor*> GunnerActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGunnerCharacter::StaticClass(), GunnerActors);
	GunnerActors.Remove(Weapon->GetGunnerCharacterOwner());
	for (AActor* GunnerActor : GunnerActors)
	{
		if (AGunnerCharacter* HitGunner = Cast<AGunnerCharacter>(GunnerActor))
		{
			if (Weapon->GetGunnerCharacterOwner()->GetLocalRole() == ROLE_Authority)
			{
				if (UPhysicsAsset* PhysAsset = HitGunner->GetMesh()->GetPhysicsAsset())
				{
					TArray<FTransform> HitBoxTransforms;
					TArray<FVector2D> Sizes;
					for (const USkeletalBodySetup* BodySetup : PhysAsset->SkeletalBodySetups)
					{
						FTransform BodyTransform = HitGunner->GetMesh()->GetSocketTransform(BodySetup->BoneName);
						for (const FKSphylElem& SphylElem : BodySetup->AggGeom.SphylElems)
						{
							FTransform HitBoxTransform = SphylElem.GetTransform() * BodyTransform;
							HitBoxTransforms.Add(HitBoxTransform);
							Sizes.Add(FVector2D(SphylElem.GetScaledHalfLength(FVector(1.0f, 1.0f, 1.0f)), SphylElem.GetScaledRadius(FVector(1.0f, 1.0f, 1.0f))));
						}
					}
					ClientDrawServerRegisteredHitBox(HitBoxTransforms, Sizes);
				}
			}
			else
			{
				if (UPhysicsAsset* PhysAsset = HitGunner->GetMesh()->GetPhysicsAsset())
				{
					for (const USkeletalBodySetup* BodySetup : PhysAsset->SkeletalBodySetups)
					{
						FTransform BodyTransform = HitGunner->GetMesh()->GetSocketTransform(BodySetup->BoneName);
						for (const FKSphylElem& SphylElem : BodySetup->AggGeom.SphylElems)
						{
							FTransform HitBoxTransform = SphylElem.GetTransform() * BodyTransform;
							DrawDebugCapsule(GetWorld(), HitBoxTransform.GetLocation(), SphylElem.GetScaledHalfLength(FVector(1.0f, 1.0f, 1.0f)), SphylElem.GetScaledRadius(FVector(1.0f, 1.0f, 1.0f)), HitBoxTransform.GetRotation(), FColor::Blue, true);
						}
					}
				}
			}
		}
	}
}

void UWeaponFireComponent::ClientDrawServerRegisteredHitBox_Implementation(const TArray<FTransform>& HitBoxTransforms, const TArray<FVector2D>& Sizes)
{
	for (int i = 0; i < HitBoxTransforms.Num(); ++i)
	{
		DrawDebugCapsule(GetWorld(), HitBoxTransforms[i].GetLocation(), Sizes[i].X, Sizes[i].Y, HitBoxTransforms[i].GetRotation(), FColor::Yellow, true);
	}
}

void UWeaponFireComponent::ServerFire_Implementation()
{
	WeaponLineTrace();
}
