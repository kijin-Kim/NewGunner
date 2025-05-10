// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerCharacter.h"

#include "CameraControlComponent.h"
#include "GunnerCharacterMovementComponent.h"
#include "Action/NexusAction.h"
#include "Action/NexusActionComponent.h"
#include "Animation/NexusAnimMontagePlayerComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SpringArmComponent.h"
#include "Gunner/Gunner.h"
#include "Gunner/Action/GunnerActionSet.h"
#include "Gunner/Item/GunnerInventoryManagerComponent.h"
#include "Gunner/_Core/GunnerBlueprintFunctionLibrary.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"
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
	FirstPersonMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->SetRenderCustomDepth(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetMesh()->SetCollisionObjectType(ECC_Pawn);
	GetMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);


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

void AGunnerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	ThirdPersonMaterialInstances.Empty();
	if (GetMesh())
	{
		for (int i = 0; i < GetMesh()->GetNumMaterials(); ++i)
		{
			UMaterialInterface* MaterialInterface = GetMesh()->GetMaterial(i);
			check(MaterialInterface);
			UMaterialInstanceDynamic* MaterialInstance = UMaterialInstanceDynamic::Create(MaterialInterface, this);
			GetMesh()->SetMaterial(i, MaterialInstance);
			ThirdPersonMaterialInstances.Add(MaterialInstance);
		}
	}
}

void AGunnerCharacter::UnPossessed()
{
	AuthRemoveActionSets();
	Super::UnPossessed();
}

void AGunnerCharacter::OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState)
{
	Super::OnPlayerStateChanged(NewPlayerState, OldPlayerState);
	// OnRep_PlayerState -> SetPlayerState -> OnPawnSet (폰과 플레이어스테이트가 서로 지정함) -> OnPlayerStateChanged 
	if (NewPlayerState)
	{
		APlayerController* PC = GetController<APlayerController>();
		if (PC && PC->IsLocalController())
		{
			CameraControllerComponent->InitCameraController();
		}

		GetCharacterMovement<UGunnerCharacterMovementComponent>()->InitEvents();
		IGunnerTeamAgentInterface* TeamAgentInterface = GetPlayerState<IGunnerTeamAgentInterface>();
		TeamAgentInterface->GetOnTeamSetDelegate()->AddUObject(this, &AGunnerCharacter::OnTeamSetEvent);
		OnTeamSetEvent(TeamAgentInterface->GetGenericTeamId(), TeamAgentInterface->GetGenericTeamId());

		if (HasAuthority())
		{
			AuthAddActionSets();
		}
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
	APlayerState* PS = GetPlayerState();
	return PS ? PS->FindComponentByClass<UNexusActionComponent>() : FindComponentByClass<UNexusActionComponent>();
}

UGunnerInventoryManagerComponent* AGunnerCharacter::GetInventoryManagerComponent() const
{
	APlayerState* PS = GetPlayerState();
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

EGunnerHitPartType AGunnerCharacter::GetHitPartTypeByHitBoneName_Implementation(FName HitBoneName) const
{
	const static TArray<FName> HeadBoneNames = {TEXT("Head"), TEXT("Neck")};
	const static TArray<FName> LegBoneNames = {TEXT("L_Hip"),TEXT("L_Knee"),TEXT("L_Foot"),TEXT("R_Hip"),TEXT("R_Knee"),TEXT("R_Foot")};
	if (HeadBoneNames.Contains(HitBoneName))
	{
		return EGunnerHitPartType::Head;
	}

	if (LegBoneNames.Contains(HitBoneName))
	{
		return EGunnerHitPartType::Leg;
	}

	return EGunnerHitPartType::Body;
}

void AGunnerCharacter::NetMulticastTriggerCue_Implementation(TSubclassOf<ANexusCue> CueClass, FNexusPredictionTag PredictionTag, const FNexusCueParameters& CueParameters)
{
	if (UNexusActionComponent* ActionComponent = GetActionComponent())
	{
		ActionComponent->SimTriggerCue(CueClass, PredictionTag, CueParameters);
	}
}


void AGunnerCharacter::AuthAddActionSets()
{
	if (!HasAuthority())
	{
		GR_LOG_SUB(this, LogGunner, Error, TEXT("권한 없는 함수 호출"));
		return;
	}

	UNexusActionComponent* ActionComponent = GetActionComponent();
	check(ActionComponent);
	for (const UGunnerActionSet* ActionSet : ActionSets)
	{
		UGunnerBlueprintFunctionLibrary::AuthAddDesiredActions(ActionComponent->GetAgentActor(), ActionComponent->GetAgentActor(), ActionSet->ActionClasses, AddedActionHandles);
		UGunnerBlueprintFunctionLibrary::AuthAddDesiredItems(ActionComponent->GetAgentActor(), ActionSet->ItemDefinitions, AddedItems);
	}
}

void AGunnerCharacter::AuthRemoveActionSets()
{
	if (!HasAuthority())
	{
		GR_LOG_SUB(this, LogGunner, Error, TEXT("권한 없는 함수 호출"));
		return;
	}

	if (UNexusActionComponent* ActionComponent = GetActionComponent())
	{
		UGunnerBlueprintFunctionLibrary::AuthRemoveDesiredActions(ActionComponent->GetAgentActor(), AddedActionHandles);
		AddedActionHandles.Empty();

		UGunnerBlueprintFunctionLibrary::AuthRemoveDesiredItems(ActionComponent->GetAgentActor(), AddedItems, true);
		AddedItems.Empty();
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
