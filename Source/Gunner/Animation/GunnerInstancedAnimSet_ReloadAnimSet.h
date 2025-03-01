// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gunner/_Core/Animation/GunnerInstancedAnimSet.h"
#include "GunnerInstancedAnimSet_ReloadAnimSet.generated.h"

/**
 * 
 */
UCLASS(DisplayName = "ReloadAnimSet")
class GUNNER_API UGunnerInstancedAnimSet_ReloadAnimSet : public UGunnerInstancedAnimSet
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> FPCharacterReloadMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> FPWeaponReloadMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> TPCharacterReloadMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> TPWeaponReloadMontage;
};