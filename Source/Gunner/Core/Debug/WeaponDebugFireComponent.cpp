// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponDebugFireComponent.h"

#include "GameFramework/GameModeBase.h"
#include "Gunner/Character/GunnerCharacter.h"
#include "Gunner/GunnerPlayerController.h"
#include "Gunner/Core/LagCompensationComponent.h"
#include "Gunner/Weapon/Weapon.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/PhysicsAsset.h"


// Sets default values for this component's properties
UWeaponDebugFireComponent::UWeaponDebugFireComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
}

void UWeaponDebugFireComponent::InitializeComponent()
{
	Super::InitializeComponent();
	AWeapon* Weapon = GetOwner<AWeapon>();
	Weapon->OnPrimaryActionDelegate.AddUniqueDynamic(this, &ThisClass::OnPrimaryAction);
}

void UWeaponDebugFireComponent::OnPrimaryAction(bool bPressed)
{
	FlushPersistentDebugLines(GetWorld());
	AWeapon* Weapon = GetOwner<AWeapon>();
	Weapon->GetGunnerCharacterOwner()->PlayAnimMontage(TPCharacterFireMontage);
	AGunnerPlayerController* GunnerPlayerController = Weapon->GetGunnerCharacterOwner()->GetController<AGunnerPlayerController>();
	WeaponLineTrace(GunnerPlayerController->GetLocalServerTime());
	if (GetOwner<AWeapon>()->GetGunnerCharacterOwner()->GetLocalRole() < ROLE_Authority)
	{
		ServerFire(GunnerPlayerController->GetLocalServerTime());
	}
}

void UWeaponDebugFireComponent::ServerFire_Implementation(double TimeStamp)
{
	WeaponLineTrace(TimeStamp);
}

void UWeaponDebugFireComponent::WeaponLineTrace(double TimeStamp)
{
	AGunnerCharacter* GunnerCharacterOwner = GetGunnerCharacterOwner();
	check(GunnerCharacterOwner);
	if (!GunnerCharacterOwner->HasAuthority())
	{
		LineTrace();
		return;
	}

	TArray<AActor*> GunnerActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGunnerCharacter::StaticClass(), GunnerActors);
	GunnerActors.Remove(GunnerCharacterOwner);


	ULagCompensationComponent* LagCompensationComponent = GetWorld()->GetAuthGameMode()->GetComponentByClass<ULagCompensationComponent>();
	LagCompensationComponent->BeginRewind(TimeStamp, GunnerActors);
	LineTrace();
	LagCompensationComponent->EndRewind();
}


void UWeaponDebugFireComponent::LineTrace()
{
	AWeapon* Weapon = GetOwner<AWeapon>();
	const AGunnerCharacter* GunnerCharacterOwner = Weapon->GetGunnerCharacterOwner();

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


		AGunnerCharacter* HitGunner = Cast<AGunnerCharacter>(Hit.GetActor());
		if(!HitGunner)
		{
			continue;
		}
		
		UPhysicsAsset* PhysAsset = HitGunner->GetMesh()->GetPhysicsAsset();
		check(PhysAsset);
		for (USkeletalBodySetup* BodySetup : PhysAsset->SkeletalBodySetups)
		{
			
			TArray<FHitBox> HitBoxes;
			FTransform BodyTransform = HitGunner->GetMesh()->GetSocketTransform(BodySetup->BoneName);
			for (FKSphylElem& SphylElem : BodySetup->AggGeom.SphylElems)
			{
				FTransform HitBoxTransform = SphylElem.GetTransform() * BodyTransform;
				HitBoxes.Add({
					.Transform = HitBoxTransform,
					.HalfHeight = SphylElem.GetScaledHalfLength(FVector(1.0f, 1.0f, 1.0f)),
					.Radius = SphylElem.GetScaledRadius(FVector(1.0f, 1.0f, 1.0f)),
					.BoneName = BodySetup->BoneName,
				});
			}
			ClientDrawHitBoxes(HitBoxes, GunnerCharacterOwner->HasAuthority() ? FColor::Red : FColor::Blue,  true, -1.0f);
		}
	}
}

void UWeaponDebugFireComponent::ClientDrawHitBoxes_Implementation(const TArray<FHitBox>& HitBoxes, FColor Color, bool bPersistentLines, float Time)
{
	for (const auto& [Transform, HalfHeight, Radius, BoneName] : HitBoxes)
	{
		DrawDebugCapsule(GetWorld(), Transform.GetLocation(), HalfHeight, Radius, Transform.GetRotation(), Color, bPersistentLines, Time);
	}
}

AGunnerCharacter* UWeaponDebugFireComponent::GetGunnerCharacterOwner() const
{
	const AWeapon* Weapon = GetOwner<AWeapon>();
	return Weapon ? Weapon->GetGunnerCharacterOwner() : nullptr;
}
