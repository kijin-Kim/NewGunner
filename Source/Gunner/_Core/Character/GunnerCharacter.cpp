// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerCharacter.h"

#include "CameraControlComponent.h"
#include "GunnerCharacterMovementComponent.h"
#include "Action/NexusActionComponent.h"
#include "Action/SubComponent/NexusCueComponent.h"
#include "Animation/NexusAnimMontagePlayerComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SpringArmComponent.h"
#include "Gunner/Item/GunnerInventoryManagerComponent.h"
#include "Gunner/_Core/Player/GunnerPlayerState.h"

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
	GetMesh()->SetRenderCustomDepth(true);


	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMeshComponent, TEXT("Socket_Camera"));
	FirstPersonCameraComponent->SetFieldOfView(71.0f);

	CameraControllerComponent = CreateDefaultSubobject<UCameraControllerComponent>(TEXT("CameraController"));
	AnimMontagePlayerComponent = CreateDefaultSubobject<UNexusAnimMontagePlayerComponent>(TEXT("AnimMontagePlayer"));
	AnimMontagePlayerComponent->SetIsReplicated(true);

	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	LagCompensationComponent = CreateDefaultSubobject<UGunnerLagCompensationComponent>(TEXT("LagCompensationComponent"));
}

void AGunnerCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	for (int i = 0; i < GetMesh()->GetNumMaterials(); ++i)
	{
		UMaterialInterface* MaterialInterface = GetMesh()->GetMaterial(i);
		check(MaterialInterface);
		UMaterialInstanceDynamic* MaterialInstance = UMaterialInstanceDynamic::Create(MaterialInterface, this);
		GetMesh()->SetMaterial(i, MaterialInstance);
		ThirdPersonMaterialInstances.Add(MaterialInstance);
	}
}




void AGunnerCharacter::OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState)
{
	Super::OnPlayerStateChanged(NewPlayerState, OldPlayerState);

	if (NewPlayerState)
	{
		if (IsLocallyControlled() && IsPlayerControlled())
		{
			CameraControllerComponent->InitCameraController();
		}

		GetCharacterMovement<UGunnerCharacterMovementComponent>()->InitEvents();
		IGunnerTeamAgentInterface* TeamAgentInterface = GetPlayerState<IGunnerTeamAgentInterface>();
		TeamAgentInterface->GetOnTeamSetDelegate()->AddUObject(this, &AGunnerCharacter::OnTeamSetEvent);
		OnTeamSetEvent(TeamAgentInterface->GetGenericTeamId(), TeamAgentInterface->GetGenericTeamId());
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

UGunnerInventoryManagerComponent* AGunnerCharacter::GetInventoryManagerComponent() const
{
	const APlayerState* PS = GetPlayerState<APlayerState>();
	return PS ? PS->FindComponentByClass<UGunnerInventoryManagerComponent>() : FindComponentByClass<UGunnerInventoryManagerComponent>();
}

void AGunnerCharacter::SetGenericTeamId(const FGenericTeamId& TeamID)
{
	IGunnerTeamAgentInterface::SetGenericTeamId(TeamID);
}

FGenericTeamId AGunnerCharacter::GetGenericTeamId() const
{
	IGunnerTeamAgentInterface* TeamAgentInterface = GetPlayerState<IGunnerTeamAgentInterface>();
	return TeamAgentInterface ? TeamAgentInterface->GetGenericTeamId() : FGenericTeamId::NoTeam;
}

ETeamAttitude::Type AGunnerCharacter::GetTeamAttitudeTowards(const AActor& Other) const
{
	IGunnerTeamAgentInterface* TeamAgentInterface = GetPlayerState<IGunnerTeamAgentInterface>();
	return TeamAgentInterface ? TeamAgentInterface->GetTeamAttitudeTowards(Other) : ETeamAttitude::Neutral;
}

FOnGunnerTeamSetSignature* AGunnerCharacter::GetOnTeamSetDelegate()
{
	IGunnerTeamAgentInterface* TeamAgentInterface = GetPlayerState<IGunnerTeamAgentInterface>();
	return TeamAgentInterface ? TeamAgentInterface->GetOnTeamSetDelegate() : nullptr;
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

void AGunnerCharacter::NetMulticastTriggerCue_Implementation(const FNexusTriggerCueParams& CueParams, FNexusLoopingCueHandle CueHandle)
{
	if (UNexusActionComponent* ActionComponent = GetActionComponent())
	{
		ActionComponent->SimTriggerCue(CueParams, CueHandle);
	}
}

void AGunnerCharacter::OnTeamSetEvent(FGenericTeamId OldTeamID, FGenericTeamId NewTeamID)
{
	GetMesh()->SetCustomDepthStencilValue(NewTeamID + 1);
	for (int i = 0; i < ThirdPersonMaterialInstances.Num(); ++i)
	{
		ThirdPersonMaterialInstances[i]->SetScalarParameterValue(FName("MyTeamId"), NewTeamID);
		ThirdPersonMaterialInstances[i]->SetScalarParameterValue(FName("UseFresnel"), 1.0f);
	}
}
