// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TraceHitMessageData.generated.h"

class AGunnerEquipment;
/**
 * 
 */
UCLASS(BlueprintType)
class GUNNER_API UGunnerHitMessageData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	AGunnerEquipment* HitEquipment;
	UPROPERTY(BlueprintReadOnly)
	FName HitBoneName;
	UPROPERTY(BlueprintReadOnly)
	FVector HitNormal;
	UPROPERTY(BlueprintReadOnly)
	float DamageAmount = 0.0f;
};
