// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Gunner.h"
#include "GunnerCharacterMovementComponent.h"
#include "HealthComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Weapon/WeaponManagerComponent.h"


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
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void AGunnerCharacter::OnFirePressed()
{
	if (OnFirePressedDelegate.IsBound())
	{
		OnFirePressedDelegate.Broadcast();
	}
}

void AGunnerCharacter::OnFireReleased()
{
	if (OnFireReleasedDelegate.IsBound())
	{
		OnFireReleasedDelegate.Broadcast();
	}
}

void AGunnerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ThisClass::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ThisClass::StopJumping);
		EnhancedInputComponent->BindAction(MoveForwardRightAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
		EnhancedInputComponent->BindAction(MoveBackwardLeftAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ThisClass::Crouch, false);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ThisClass::UnCrouch, false);
		EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Started, this, &ThisClass::SetRunning, false);
		EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Completed, this, &ThisClass::SetRunning, true);
		
		EnhancedInputComponent->BindAction(PrimaryWeaponEquipAction, ETriggerEvent::Triggered, WeaponManagerComponent.Get(), &UWeaponManagerComponent::ChangeCurrentWeapon, static_cast<uint32>(0));
		EnhancedInputComponent->BindAction(SecondaryWeaponEquipAction, ETriggerEvent::Triggered, WeaponManagerComponent.Get(), &UWeaponManagerComponent::ChangeCurrentWeapon, static_cast<uint32>(1));
		EnhancedInputComponent->BindAction(MeleeWeaponEquipAction, ETriggerEvent::Triggered, WeaponManagerComponent.Get(), &UWeaponManagerComponent::ChangeCurrentWeapon, static_cast<uint32>(2));
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ThisClass::OnFirePressed);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ThisClass::OnFireReleased);
	}
}


void AGunnerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	SetupMappingContext();
}

void AGunnerCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();
	SetupMappingContext();
}

bool AGunnerCharacter::IsRunning() const
{
	return bIsRunning;
}

void AGunnerCharacter::SetRunning(bool bNewRunning)
{
	bIsRunning = bNewRunning;
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerRun(bNewRunning);	
	}
}

void AGunnerCharacter::ServerRun_Implementation(bool bNewRunning)
{
	SetRunning(bNewRunning);
}

void AGunnerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AGunnerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X * BaseTurnRate * MouseSensitivity);
		AddControllerPitchInput(LookAxisVector.Y * BaseTurnRate * MouseSensitivity);
	}
}

void AGunnerCharacter::SetupMappingContext()
{
	const APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (PlayerController && PlayerController->IsLocalController())
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		check(Subsystem);
		check(DefaultMappingContext);
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}
