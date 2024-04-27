// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponFireComponent.h"

#include "Weapon.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerState.h"
#include "Gunner/Gunner.h"
#include "Gunner/GunnerCharacter.h"
#include "Gunner/GunnerPlayerController.h"
#include "Gunner/Core/LagCompensationComponent.h"
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

void UWeaponFireComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
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
	AGunnerPlayerController* GunnerPlayerController = Weapon->GetGunnerCharacterOwner()->GetController<AGunnerPlayerController>();
	WeaponLineTrace(GunnerPlayerController->GetLocalServerTime());
	if (GetOwner<AWeapon>()->GetGunnerCharacterOwner()->GetLocalRole() < ROLE_Authority)
	{
		ServerFire(GunnerPlayerController->GetLocalServerTime());
	}
}

void UWeaponFireComponent::WeaponLineTrace(double TimeStamp)
{
	TArray<FHitResult> HitResults;
	AWeapon* Weapon = GetOwner<AWeapon>();
	AGunnerCharacter* GunnerCharacterOwner = Weapon->GetGunnerCharacterOwner();
	APlayerCameraManager* PlayerCameraManager = Cast<APlayerController>(GunnerCharacterOwner->GetController())->PlayerCameraManager;
	FVector CameraLocation = PlayerCameraManager->GetCameraLocation();
	FVector Forward = PlayerCameraManager->GetCameraRotation().Vector();

	FCollisionResponseParams ResponseParams;
	ResponseParams.CollisionResponse.SetAllChannels(ECR_Overlap);
	GetWorld()->LineTraceMultiByChannel(HitResults, CameraLocation, CameraLocation + Forward * 100000.0f, ECC_Visibility, FCollisionQueryParams::DefaultQueryParam, ResponseParams);


	TArray<AActor*> AlreadyHitRegisteredActors;
	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor == Weapon || HitActor == GunnerCharacterOwner || AlreadyHitRegisteredActors.Find(HitActor) != INDEX_NONE)
		{
			continue;
		}


		AlreadyHitRegisteredActors.Add(HitActor);
		UGameplayStatics::ApplyPointDamage(HitActor, 10.0f, Forward, Hit, GunnerCharacterOwner->GetController(), Weapon, UDamageType::StaticClass());
	}


	TArray<AActor*> GunnerActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGunnerCharacter::StaticClass(), GunnerActors);
	GunnerActors.Remove(GunnerCharacterOwner);
	for (AActor* GunnerActor : GunnerActors)
	{
		AGunnerCharacter* HitGunner = Cast<AGunnerCharacter>(GunnerActor);
		check(HitGunner);
		UPhysicsAsset* PhysAsset = HitGunner->GetMesh()->GetPhysicsAsset();
		check(PhysAsset);

		TArray<FHitBox> HitBoxes;
		for (const USkeletalBodySetup* BodySetup : PhysAsset->SkeletalBodySetups)
		{
			FTransform BodyTransform = HitGunner->GetMesh()->GetSocketTransform(BodySetup->BoneName);
			for (const FKSphylElem& SphylElem : BodySetup->AggGeom.SphylElems)
			{
				FTransform HitBoxTransform = SphylElem.GetTransform() * BodyTransform;
				if (GunnerCharacterOwner->GetLocalRole() == ROLE_Authority)
				{
					HitBoxes.Add({
						.Transform = HitBoxTransform,
						.HalfHeight = SphylElem.GetScaledHalfLength(FVector(1.0f, 1.0f, 1.0f)),
						.Radius = SphylElem.GetScaledRadius(FVector(1.0f, 1.0f, 1.0f)),
						.BoneName = BodySetup->BoneName,
					});
				}
				else
				{
					DrawDebugCapsule(GetWorld(), HitBoxTransform.GetLocation(), SphylElem.GetScaledHalfLength(FVector(1.0f, 1.0f, 1.0f)), SphylElem.GetScaledRadius(FVector(1.0f, 1.0f, 1.0f)), HitBoxTransform.GetRotation(), FColor::Blue, true);
				}
			}

			if (GunnerCharacterOwner->GetLocalRole() == ROLE_Authority)
			{
				//ClientDrawServerRegisteredHitBox(HitBoxes);
			}
		}
	}

	if (GunnerCharacterOwner->HasAuthority())
	{
		double SingleTripTimeFromClient = GetWorld()->GetTimeSeconds() - TimeStamp;
		double AdjustedClientTimeStamp = TimeStamp - SingleTripTimeFromClient; // 클라이언트가 Remote Client에 대해 서버보다 과거 위치를 보는 것을 반영합니다.
		double TargetTime = FMath::Min(GetWorld()->GetTimeSeconds(), AdjustedClientTimeStamp);
		const auto& Hists = GetWorld()->GetAuthGameMode()->GetComponentByClass<ULagCompensationComponent>()->HitBoxHistories;
		if (!Hists.IsEmpty())
		{
			for (AActor* GunnerActor : GunnerActors)
			{
				ClientDrawServerRegisteredHitBox(Hists.First().HitBoxes[GunnerActor], FColor::Magenta);
				ClientDrawServerRegisteredHitBox(Hists.Last().HitBoxes[GunnerActor], FColor::Red);
			}
		}

		for (const auto& History : Hists)
		{
			if(History.Time <= TargetTime)
			{
				for (AActor* GunnerActor : GunnerActors)
				{
					ClientDrawServerRegisteredHitBox(History.HitBoxes[GunnerActor], FColor::Orange);
				}
				break;
			}
		}
	}
}

void UWeaponFireComponent::ClientDrawServerRegisteredHitBox_Implementation(const TArray<FHitBox>& HitBoxes, FColor Color)
{
	for (const auto& [Transform, HalfHeight, Radius, BoneName] : HitBoxes)
	{
		DrawDebugCapsule(GetWorld(), Transform.GetLocation(), HalfHeight, Radius, Transform.GetRotation(), Color, true);
	}
}

void UWeaponFireComponent::ServerFire_Implementation(double TimeStamp)
{
	WeaponLineTrace(TimeStamp);
}
