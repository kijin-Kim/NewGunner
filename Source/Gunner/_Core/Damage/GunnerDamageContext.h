// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GunnerDamageContext.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct GUNNER_API FGunnerDamageContext
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> Instigator;
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> Causer;
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> Target;
	UPROPERTY(BlueprintReadOnly)
	FVector HitNormal;
	UPROPERTY(BlueprintReadOnly)
	FName HitBoneName;
	UPROPERTY(BlueprintReadOnly)
	bool bIsAlt = false;
};
