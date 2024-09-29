// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerCharacter.h"

#include "CameraControlComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GunnerCharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SpringArmComponent.h"
#include "Gunner/Gunner.h"
#include "Gunner/Core/AnimMontagePlayerComponent.h"
#include "Gunner/Core/ActionSystem/GunnerActionComponent.h"
#include "Gunner/Core/Event/EventManagerComponent.h"
#include "Gunner/Equipment/GunnerEquipment.h"
#include "Gunner/Equipment/GunnerEquipmentManagerComponent.h"
#include "Gunner/Weapon/Weapon.h"
#include "Gunner/Weapon/WeaponManagerComponent.h"

AGunnerCharacter::AGunnerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UGunnerCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	GetCapsuleComponent()->SetCapsuleHalfHeight(98.0f);
	GetCapsuleComponent()->SetCapsuleRadius(42.0f);


	FirstPersonSpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("FirstPersonSpringArm"));
	FirstPersonSpringArmComponent->SetupAttachment(GetRootComponent());
	FirstPersonSpringArmComponent->TargetArmLength = 0.0f;
	FirstPersonSpringArmComponent->bDoCollisionTest = false;
	FirstPersonSpringArmComponent->bUsePawnControlRotation = true;

	FirstPersonMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMeshComponent->SetupAttachment(FirstPersonSpringArmComponent);
	FirstPersonMeshComponent->SetOnlyOwnerSee(true);
	GetMesh()->SetOwnerNoSee(true);


	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMeshComponent, TEXT("CameraSocket"));
	FirstPersonCameraComponent->SetFieldOfView(71.0f);

	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	WeaponManagerComponent = CreateDefaultSubobject<UWeaponManagerComponent>(TEXT("WeaponManager"));
	AnimMontagePlayerComponent = CreateDefaultSubobject<UAnimMontagePlayerComponent>(TEXT("AnimMontagePlayer"));


	CameraControllerComponent = CreateDefaultSubobject<UCameraControllerComponent>(TEXT("CameraController"));

	EquipmentManagerComponent = CreateDefaultSubobject<UGunnerEquipmentManagerComponent>(TEXT("EquipmentManager"));
}

void AGunnerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Started, this, &ThisClass::SetRunning, false);
		EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Completed, this, &ThisClass::SetRunning, true);
	}
	//WeaponManagerComponent->SetupPlayerInputComponent(PlayerInputComponent);
}


bool AGunnerCharacter::CanJumpInternal_Implementation() const
{
	return JumpIsAllowedInternal();
}

UAnimMontagePlayerComponent* AGunnerCharacter::GetAnimMontagePlayer_Implementation()
{
	return AnimMontagePlayerComponent;
}

bool AGunnerCharacter::IsRunning() const
{
	return bIsRunning;
}

UGunnerActionComponent* AGunnerCharacter::GetActionComponent() const
{
	const APlayerState* PS = GetPlayerState<APlayerState>();
	return PS ? PS->FindComponentByClass<UGunnerActionComponent>() : FindComponentByClass<UGunnerActionComponent>();
}

void AGunnerCharacter::OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState)
{
	Super::OnPlayerStateChanged(NewPlayerState, OldPlayerState);
	GR_LOG(LogGunner, Warning, TEXT(""));

	if (NewPlayerState)
	{
		GetActionComponent()->InitActionComponent(NewPlayerState, this);
	}


	if (HasAuthority() && NewPlayerState && NewPlayerState != OldPlayerState)
	{
		check(InitialEquipmentClasses.Num() <= 3);
		for (int i = 0; i < InitialEquipmentClasses.Num(); ++i)
		{
			if (InitialEquipmentClasses[i])
			{
				EquipmentManagerComponent->AuthAddEquipmentToSlot(i, InitialEquipmentClasses[i]);
			}
		}
	}
}

void AGunnerCharacter::SetRunning(bool bNewRunning)
{
	bIsRunning = bNewRunning;
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerRun(bNewRunning);
	}
}

void AGunnerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	// if (HasAuthority())
	// {
	// 	if (TestWeaponClass)
	// 	{
	// 		TestWeapon = GetWorld()->SpawnActorDeferred<AWeapon>(TestWeaponClass, GetTransform());
	// 		TestWeapon->SetOwner(this);
	// 		TestWeapon->SetInstigator(this);
	// 		TestWeapon->SetAutonomousProxy(true);
	// 		TestWeapon->FinishSpawning(GetTransform());
	// 	}
	// }
}

void AGunnerCharacter::ServerRun_Implementation(bool bNewRunning)
{
	SetRunning(bNewRunning);
}
