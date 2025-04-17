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
};
