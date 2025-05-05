// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GunnerHitBoxInterface.generated.h"

UENUM()
enum class EGunnerHitPartType
{
	Head,
	Body,
	Leg
};

UENUM()
enum class EGunnerHitDirectionType
{
	Front,
	Back,
	Left,
	Right,
};


// This class does not need to be modified.
UINTERFACE()
class UGunnerHitBoxInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GUNNER_API IGunnerHitBoxInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	EGunnerHitPartType GetHitPartTypeByHitBoneName(FName HitBoneName) const;
	static EGunnerHitDirectionType GetHitDirectionType(const FVector& CauserLocation, const FVector& VictimLocation, const FVector& VictimForward);
};
