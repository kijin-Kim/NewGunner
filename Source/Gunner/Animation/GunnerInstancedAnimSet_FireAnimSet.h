// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gunner/_Core/Animation/GunnerInstancedAnimSet.h"
#include "GunnerInstancedAnimSet_FireAnimSet.generated.h"

/**
 * 
 */
UCLASS(DisplayName = "FireAnimSet")
class GUNNER_API UGunnerInstancedAnimSet_FireAnimSet : public UGunnerInstancedAnimSet
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> FPCharacterFireMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> FPWeaponFireMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> TPCharacterFireMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> TPWeaponFireMontage;
};