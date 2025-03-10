// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerCharacter.h"

#include "CameraControlComponent.h"
#include "GunnerCharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SpringArmComponent.h"
#include "Animation/NexusAnimMontagePlayerComponent.h"
#include "Gunner/Equipment/GunnerEquipmentManagerComponent.h"
#include "Gunner/Action/GunnerActionSetupComponent.h"
#include "NexusActionComponent.h"
#include "Event/NexusEventManagerComponent.h"

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
	FirstPersonMeshComponent->bOnlyOwnerSee = true;
	FirstPersonMeshComponent->CastShadow = false;
	FirstPersonMeshComponent->bRenderCustomDepth = true;
	GetMesh()->SetOwnerNoSee(true);


	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMeshComponent, TEXT("CameraSocket"));
	FirstPersonCameraComponent->SetFieldOfView(71.0f);

	CameraControllerComponent = CreateDefaultSubobject<UCameraControllerComponent>(TEXT("CameraController"));
	AnimMontagePlayerComponent = CreateDefaultSubobject<UNexusAnimMontagePlayerComponent>(TEXT("AnimMontagePlayer"));
	EquipmentManagerComponent = CreateDefaultSubobject<UGunnerEquipmentManagerComponent>(TEXT("EquipmentManager"));

	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	LagCompensationComponent = CreateDefaultSubobject<UGunnerLagCompensationComponent>(TEXT("LagCompensationComponent"));
	ActionSetupComponent = CreateDefaultSubobject<UGunnerActionSetupComponent>(TEXT("ActionSetupComponent"));
}

void AGunnerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (EquipmentManagerComponent && HasAuthority())
	{
		EquipmentManagerComponent->AuthRelaseEquipmentManagerComponent();
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
			EquipmentManagerComponent->AuthInitEquipmentManagerComponent();
		}

		CameraControllerComponent->InitCameraController();
		GetCharacterMovement<UGunnerCharacterMovementComponent>()->InitEvents();

		// TODO: Callon match start
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			GetOnTeamSetDelegate()->AddUObject(this, &AGunnerCharacter::OnTeamSetEvent);
			OnTeamSetEvent(GetGenericTeamId(), GetGenericTeamId());
		}, 1.0f, false);
	}
}

bool AGunnerCharacter::CanJumpInternal_Implementation() const
{
	return JumpIsAllowedInternal();
}

UNexusAnimMontagePlayerComponent* AGunnerCharacter::GetAnimMontagePlayer_Implementation()
{
	return AnimMontagePlayerComponent;
}


UNexusActionComponent* AGunnerCharacter::GetActionComponent() const
{
	const APlayerState* PS = GetPlayerState<APlayerState>();
	return PS ? PS->FindComponentByClass<UNexusActionComponent>() : FindComponentByClass<UNexusActionComponent>();
}

UNexusEventManagerComponent* AGunnerCharacter::GetEventManagerComponent() const
{
	const APlayerState* PS = GetPlayerState<APlayerState>();
	return PS ? PS->FindComponentByClass<UNexusEventManagerComponent>() : FindComponentByClass<UNexusEventManagerComponent>();
}

void AGunnerCharacter::SetGenericTeamId(const FGenericTeamId& TeamID)
{
	if (IGunnerTeamAgentInterface* PS = GetPlayerState<IGunnerTeamAgentInterface>())
	{
		PS->SetGenericTeamId(TeamID);
	}
}

FGenericTeamId AGunnerCharacter::GetGenericTeamId() const
{
	if (const IGunnerTeamAgentInterface* PS = GetPlayerState<IGunnerTeamAgentInterface>())
	{
		return PS->GetGenericTeamId();
	}
	return FGenericTeamId::NoTeam;
}

ETeamAttitude::Type AGunnerCharacter::GetTeamAttitudeTowards(const AActor& Other) const
{
	if (const IGunnerTeamAgentInterface* PS = GetPlayerState<IGunnerTeamAgentInterface>())
	{
		return PS->GetTeamAttitudeTowards(Other);
	}
	return ETeamAttitude::Neutral;
}

FOnGunnerTeamSetSignature* AGunnerCharacter::GetOnTeamSetDelegate()
{
	if (IGunnerTeamAgentInterface* PS = GetPlayerState<IGunnerTeamAgentInterface>())
	{
		return PS->GetOnTeamSetDelegate();
	}
	return nullptr;
}

EGunnerHitBoxType AGunnerCharacter::GetHitBoxTypeByHitBoneName_Implementation(FName HitBoneName) const
{
	const static TArray<FName> HeadBoneNames = {TEXT("Head"), TEXT("Neck")};
	const static TArray<FName> LegBoneNames = {TEXT("L_Hip"),TEXT("L_Knee"),TEXT("L_Foot"),TEXT("R_Hip"),TEXT("R_Knee"),TEXT("R_Foot")};
	if (HeadBoneNames.Contains(HitBoneName))
	{
		return EGunnerHitBoxType::Head;
	}

	if (LegBoneNames.Contains(HitBoneName))
	{
		return EGunnerHitBoxType::Leg;
	}

	return EGunnerHitBoxType::Body;
}

void AGunnerCharacter::NetMulticastTriggerCue_Implementation(TSubclassOf<UNexusCue> CueClass, FNexusTargetDataHandle TargetDataHandle, FNexusPredictionTag PredictionTag)
{
	UNexusActionComponent* ActionComponent = GetActionComponent();
	if (ActionComponent->GetOwner()->HasAuthority() || !PredictionTag.IsPredictable())
	{
		ActionComponent->InternalTriggerCue(CueClass, TargetDataHandle);
	}
}

void AGunnerCharacter::OnTeamSetEvent(FGenericTeamId OldTeamID, FGenericTeamId NewTeamID)
{
	if (!IsLocallyControlled())
	{
		for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PlayerController = Iterator->Get();
			if (PlayerController && PlayerController->IsLocalPlayerController())
			{
				IGunnerTeamAgentInterface* TeamAgentInterface = PlayerController->GetPlayerState<IGunnerTeamAgentInterface>();
				ETeamAttitude::Type Attitude = TeamAgentInterface->GetTeamAttitudeTowards(*this);
				if (Attitude == ETeamAttitude::Friendly)
				{
					GetMesh()->SetRenderCustomDepth(true);
					GetMesh()->SetCustomDepthStencilValue(1);
				}
				else
				{
					GetMesh()->SetRenderCustomDepth(false);
				}
			}
		}
	}
}
