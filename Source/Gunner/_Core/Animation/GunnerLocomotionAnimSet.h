// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AimOffsetBlendSpace.h"
#include "Engine/DataAsset.h"
#include "GunnerLocomotionAnimSet.generated.h"

USTRUCT(BlueprintType)
struct FGunnerFirstPersonAnimSet
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson")
	TObjectPtr<UAnimSequence> FPCharacterIdlePose;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson")
	TObjectPtr<UAnimSequence> FPCharacterIdleAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson")
	TObjectPtr<UAnimSequence> FPCharacterWalkAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson")
	TObjectPtr<UAnimSequence> FPCharacterRunAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson")
	TObjectPtr<UAnimSequence> FPCharacterCrouchIn;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson")
	TObjectPtr<UAnimSequence> FPCharacterCrouchOut;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson")
	TObjectPtr<UAnimSequence> FPCharacterCrouchWalkAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson")
	TObjectPtr<UAnimSequence> FPCharacterJumpAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson")
	TObjectPtr<UAnimSequence> FPCharacterJumpLandAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson")
	TObjectPtr<UAimOffsetBlendSpace> FPCharacterAimOffset;
};

USTRUCT(BlueprintType)
struct FGunnerThirdPersonAnimSet
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson")
	TObjectPtr<UAnimSequence> TPCharacterIdlePoseUB;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson")
	TObjectPtr<UAnimSequence> TPCharacterCrouchPoseUB;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson")
	TObjectPtr<UAimOffsetBlendSpace> TPCharacterAimOffsetUB;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson")
	TObjectPtr<UAimOffsetBlendSpace> TPCharacterCrouchAimOffsetUB;
};

USTRUCT(BlueprintType)
struct FGunnerAnimSet
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGunnerFirstPersonAnimSet FirstPersonAnimSet;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGunnerThirdPersonAnimSet ThirdPersonAnimSet;
};


/**
 * 
 */
UCLASS(BlueprintType)
class GUNNER_API UGunnerLocomotionAnimSet : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGunnerAnimSet PrimaryAnimSet;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGunnerAnimSet AltAnimSet;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Primary")
	FGunnerFirstPersonAnimSet FirstPersonPrimaryAnimSet;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Primary")
	FGunnerThirdPersonAnimSet ThirdPersonPrimaryAnimSet;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Alt")
	FGunnerFirstPersonAnimSet FirstPersonAltAnimSet;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Alt")
	FGunnerThirdPersonAnimSet ThirdPersonAltAnimSet;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character|Primary")
	TObjectPtr<UAnimSequence> FPCharacterIdlePose;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character|Primary")
	TObjectPtr<UAnimSequence> FPCharacterIdleAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character|Primary")
	TObjectPtr<UAnimSequence> FPCharacterWalkAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character|Primary")
	TObjectPtr<UAnimSequence> FPCharacterRunAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character|Primary")
	TObjectPtr<UAnimSequence> FPCharacterCrouchIn;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character|Primary")
	TObjectPtr<UAnimSequence> FPCharacterCrouchOut;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character|Primary")
	TObjectPtr<UAnimSequence> FPCharacterCrouchWalkAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character|Primary")
	TObjectPtr<UAnimSequence> FPCharacterJumpAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character|Primary")
	TObjectPtr<UAnimSequence> FPCharacterJumpLandAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character|Primary")
	TObjectPtr<UAimOffsetBlendSpace> FPCharacterAimOffset;
	

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterAltIdlePose;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterAltIdleAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterAltWalkAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterAltRunAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterAltCrouchIn;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterAltCrouchOut;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterAltCrouchWalkAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterAltJumpAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterAltJumpLandAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAimOffsetBlendSpace> FPCharacterAltAimOffset;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson|Character")
	TObjectPtr<UAnimSequence> TPCharacterIdlePoseUB;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson|Character")
	TObjectPtr<UAnimSequence> TPCharacterCrouchPoseUB;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson|Character")
	TObjectPtr<UAimOffsetBlendSpace> TPCharacterAimOffsetUB;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson|Character")
	TObjectPtr<UAimOffsetBlendSpace> TPCharacterCrouchAimOffsetUB;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson|Character")
	TObjectPtr<UAnimSequence> TPCharacterAltIdlePoseUB;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson|Character")
	TObjectPtr<UAnimSequence> TPCharacterAltCrouchPoseUB;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson|Character")
	TObjectPtr<UAimOffsetBlendSpace> TPCharacterAltAimOffsetUB;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson|Character")
	TObjectPtr<UAimOffsetBlendSpace> TPCharacterCrouchAltAimOffsetUB;
};
