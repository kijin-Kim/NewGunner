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
#include "Gunner/Animation/GunnerAnimMontagePlayerComponent.h"
#include "Gunner/_Core/ActionSystem/GunnerActionComponent.h"
#include "Gunner/_Core/Event/GunnerEventManagerComponent.h"
#include "Gunner/Equipment/GunnerEquipment.h"
#include "Gunner/Equipment/GunnerEquipmentManagerComponent.h"
#include "Gunner/_Core/ActionSystem/GunnerAction.h"

AGunnerCharacter::AGunnerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UGunnerCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	GetCapsuleComponent()->SetCapsuleHalfHeight(98.0f);
	GetCapsuleComponent()->SetCapsuleRadius(42.0f);

	FirstPersonMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMeshComponent->SetupAttachment(FirstPersonSpringArmComponent);
	FirstPersonMeshComponent->SetOnlyOwnerSee(true);
	GetMesh()->SetOwnerNoSee(true);

	FirstPersonSpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("FirstPersonSpringArm"));
	FirstPersonSpringArmComponent->SetupAttachment(GetRootComponent());
	FirstPersonSpringArmComponent->TargetArmLength = 0.0f;
	FirstPersonSpringArmComponent->bDoCollisionTest = false;
	FirstPersonSpringArmComponent->bUsePawnControlRotation = true;

	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMeshComponent, TEXT("CameraSocket"));
	FirstPersonCameraComponent->SetFieldOfView(71.0f);

	CameraControllerComponent = CreateDefaultSubobject<UCameraControllerComponent>(TEXT("CameraController"));
	AnimMontagePlayerComponent = CreateDefaultSubobject<UGunnerAnimMontagePlayerComponent>(TEXT("AnimMontagePlayer"));
	EquipmentManagerComponent = CreateDefaultSubobject<UGunnerEquipmentManagerComponent>(TEXT("EquipmentManager"));

	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
}

void AGunnerCharacter::OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState)
{
	Super::OnPlayerStateChanged(NewPlayerState, OldPlayerState);
	GR_LOG(LogGunner, Warning, TEXT(""));

	if (NewPlayerState)
	{
		UGunnerActionComponent* ActionComponent = GetActionComponent();
		check(ActionComponent);
		ActionComponent->InitActionComponent(NewPlayerState, this);
		for (TSubclassOf<UGunnerAction> ActionClass : InitialActions)
		{
			if (ActionClass)
			{
				FGunnerActionDefinition ActionDefinition(this, ActionClass);
				ActionComponent->AuthAddAction(ActionDefinition);
			}
		}
	}


	if (HasAuthority() && NewPlayerState && NewPlayerState != OldPlayerState)
	{
		EquipmentManagerComponent->InitEquipmentManagerComponent();
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

bool AGunnerCharacter::IsRunning() const
{
	return bIsRunning;
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
