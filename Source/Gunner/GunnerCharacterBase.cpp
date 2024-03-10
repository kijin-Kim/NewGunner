// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerCharacterBase.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Gunner.h"
#include "GunnerCharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Weapon/WeaponManagerComponent.h"


AGunnerCharacterBase::AGunnerCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UGunnerCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	

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

	GetCharacterMovement()->MaxWalkSpeed = 675.0f;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 250.0f;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	WeaponManagerComponent = CreateDefaultSubobject<UWeaponManagerComponent>(TEXT("WeaponManager"));
}


void AGunnerCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ThisClass::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ThisClass::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ThisClass::Crouch, false);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ThisClass::UnCrouch, false);
		EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Started, this, &ThisClass::Walk);
		EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Completed, this, &ThisClass::Run);
		EnhancedInputComponent->BindAction(PrimaryWeaponEquipAction, ETriggerEvent::Triggered, WeaponManagerComponent.Get(), &UWeaponManagerComponent::ChangeCurrentWeapon, static_cast<uint32>(0));
		EnhancedInputComponent->BindAction(SecondaryWeaponEquipAction, ETriggerEvent::Triggered, WeaponManagerComponent.Get(), &UWeaponManagerComponent::ChangeCurrentWeapon, static_cast<uint32>(1));
		EnhancedInputComponent->BindAction(MeleeWeaponEquipAction, ETriggerEvent::Triggered, WeaponManagerComponent.Get(), &UWeaponManagerComponent::ChangeCurrentWeapon, static_cast<uint32>(2));
	}
}

void AGunnerCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	MaxWalkSpeedCache = GetCharacterMovement()->MaxWalkSpeed;
}

void AGunnerCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	SetupMappingContext();
}

void AGunnerCharacterBase::OnRep_Controller()
{
	Super::OnRep_Controller();
	SetupMappingContext();
}

bool AGunnerCharacterBase::CanJumpInternal_Implementation() const
{
	return JumpIsAllowedInternal();
}

void AGunnerCharacterBase::Walk()
{
	if (!HasAuthority() && IsLocallyControlled())
	{
		LocalWalk();
	}

	ServerWalk();
}

void AGunnerCharacterBase::LocalWalk()
{
	GetCharacterMovement()->MaxWalkSpeed = MaxWalkSpeedCache * WalkSpeedMultiplier;
}

void AGunnerCharacterBase::ServerWalk_Implementation()
{
	LocalWalk();
}

void AGunnerCharacterBase::Run()
{
	if (!HasAuthority() && IsLocallyControlled())
	{
		LocalRun();
	}

	ServerRun();
}

void AGunnerCharacterBase::LocalRun()
{
	GetCharacterMovement()->MaxWalkSpeed = MaxWalkSpeedCache;
}

void AGunnerCharacterBase::ServerRun_Implementation()
{
	LocalRun();
}


void AGunnerCharacterBase::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AGunnerCharacterBase::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X * BaseTurnRate * MouseSensitivity);
		AddControllerPitchInput(LookAxisVector.Y * BaseTurnRate * MouseSensitivity);
	}
}

void AGunnerCharacterBase::SetupMappingContext()
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
