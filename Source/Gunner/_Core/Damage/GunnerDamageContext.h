// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GunnerDamageContext.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class GUNNER_API UGunnerDamageContext : public UObject // EventMessage의 EventDataObject로 사용하기 위해 UObject를 상속받음
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	float DamageAmount = 0.0f;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AController> Instigator;
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
