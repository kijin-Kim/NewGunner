// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gunner/_Core/Animation/GunnerInstancedAnimSet.h"
#include "GunnerInstancedAnimSet_KnifeAttackAnimSet.generated.h"

/**
 * 
 */
UCLASS(DisplayName = "KnifeAttackAnimSet")
class GUNNER_API UGunnerInstancedAnimSet_KnifeAttackAnimSet : public UGunnerInstancedAnimSet
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<UAnimMontage>> FPKnifeAttackMontages;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<UAnimMontage>> TPKnifeAttackMontages;
};
