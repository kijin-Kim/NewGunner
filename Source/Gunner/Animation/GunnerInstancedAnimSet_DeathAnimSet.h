// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gunner/_Core/GunnerHitBoxInterface.h"
#include "Gunner/_Core/GunnerDirectionalMontage.h"
#include "Gunner/_Core/Animation/GunnerInstancedAnimSet.h"
#include "GunnerInstancedAnimSet_DeathAnimSet.generated.h"

/**
 * 
 */
UCLASS(DisplayName = "DeathAnimSet")
class GUNNER_API UGunnerInstancedAnimSet_DeathAnimSet : public UGunnerInstancedAnimSet
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TMap<EGunnerHitBoxType, FGunnerDirectionalMontageSet> DeathDirectionalMontages;
};