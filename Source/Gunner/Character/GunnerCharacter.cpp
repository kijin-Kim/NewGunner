// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerCharacter.h"

#include "CameraControlComponent.h"
#include "GunnerCharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SpringArmComponent.h"
#include "Gunner/Animation/GunnerAnimMontagePlayerComponent.h"
#include "Gunner/Equipment/GunnerEquipmentManagerComponent.h"
#include "Gunner/_Core/GunnerActionSetupComponent.h"
#include "Gunner/_Core/ActionSystem/GunnerActionComponent.h"
#include "Gunner/_Core/Event/GunnerEventManagerComponent.h"

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
	FirstPersonMeshComponent->SetCastShadow(false);
	GetMesh()->SetOwnerNoSee(true);


	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMeshComponent, TEXT("CameraSocket"));
	FirstPersonCameraComponent->SetFieldOfView(71.0f);

	CameraControllerComponent = CreateDefaultSubobject<UCameraControllerComponent>(TEXT("CameraController"));
	AnimMontagePlayerComponent = CreateDefaultSubobject<UGunnerAnimMontagePlayerComponent>(TEXT("AnimMontagePlayer"));
	EquipmentManagerComponent = CreateDefaultSubobject<UGunnerEquipmentManagerComponent>(TEXT("EquipmentManager"));

	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	LagCompensationComponent = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensationComponent"));
	ActionSetupComponent = CreateDefaultSubobject<UGunnerActionSetupComponent>(TEXT("ActionSetupComponent"));
}

void AGunnerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (EquipmentManagerComponent)
	{
		EquipmentManagerComponent->RelaseEquipmentManagerComponent();
	}
	Super::EndPlay(EndPlayReason);
}

void AGunnerCharacter::OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState)
{
	Super::OnPlayerStateChanged(NewPlayerState, OldPlayerState);

	if (NewPlayerState)
	{
		if (HasAuthority())
		{
			ActionSetupComponent->AuthSetupActionSets();
		}

		EquipmentManagerComponent->InitEquipmentManagerComponent();
		CameraControllerComponent->InitCameraController();
		GetCharacterMovement<UGunnerCharacterMovementComponent>()->InitEvents();
	}
}

bool AGunnerCharacter::CanJumpInternal_Implementation() const
{
	return JumpIsAllowedInternal();
}

UGunnerAnimMontagePlayerComponent* AGunnerCharacter::GetAnimMontagePlayer_Implementation()
{
	return AnimMontagePlayerComponent;
}


UGunnerActionComponent* AGunnerCharacter::GetActionComponent() const
{
	const APlayerState* PS = GetPlayerState<APlayerState>();
	return PS ? PS->FindComponentByClass<UGunnerActionComponent>() : FindComponentByClass<UGunnerActionComponent>();
}

UGunnerEventManagerComponent* AGunnerCharacter::GetEventManagerComponent() const
{
	const APlayerState* PS = GetPlayerState<APlayerState>();
	return PS ? PS->FindComponentByClass<UGunnerEventManagerComponent>() : FindComponentByClass<UGunnerEventManagerComponent>();
}
