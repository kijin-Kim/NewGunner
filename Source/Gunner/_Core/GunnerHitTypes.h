// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerHitTypes.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EGunnerHitDirectionType : uint8
{
	Front,
	Back,
	Left,
	Right,
};

UENUM(BlueprintType)
enum class EGunnerHitBoneType : uint8
{
	None UMETA(DisplayName = "None"),
	Head UMETA(DisplayName = "Head"),
	Body UMETA(DisplayName = "Body"),
	Leg UMETA(DisplayName = "Leg"),
};

USTRUCT(BlueprintType)
struct FGunnerDirectionalMontage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> Front;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> Back;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> Left;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> Right;
};

USTRUCT(BlueprintType)
struct FGunnerDirectionalMontageSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FGunnerDirectionalMontage> MontageSet;
};