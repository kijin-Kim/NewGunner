// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AimOffsetBlendSpace.h"
#include "Engine/DataAsset.h"
#include "GunnerLocomotionAnimSet.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class GUNNER_API UGunnerLocomotionAnimSet : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterIdlePose;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterIdleAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterWalkAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterRunAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterCrouchIn;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterCrouchOut;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterCrouchWalkAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterJumpAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAnimSequence> FPCharacterJumpLandAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FirstPerson|Character")
	TObjectPtr<UAimOffsetBlendSpace> FPCharacterAimOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson|Character")
	TObjectPtr<UAnimSequence> TPCharacterIdlePoseUB;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson|Character")
	TObjectPtr<UAnimSequence> TPCharacterCrouchPoseUB;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson|Character")
	TObjectPtr<UAimOffsetBlendSpace> TPCharacterAimOffsetUB;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson|Character")
	TObjectPtr<UAimOffsetBlendSpace> TPCharacterCrouchAimOffsetUB;
};
