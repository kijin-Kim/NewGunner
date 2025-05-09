// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gunner/_Core/Damage/GunnerDamageContext.h"
#include "Gunner/_Core/Damage/GunnerDamageType.h"
#include "GunnerDamageType_FrontOrBack.generated.h"

/**
 * 
 */

UCLASS(DisplayName = "Front Or Back")
class UGunnerDamageType_FrontOrBack : public UGunnerDamageType
{
	GENERATED_BODY()

public:
	UGunnerDamageType_FrontOrBack()
		: BackDamageMultiplier(1.0f)
	{
	}

	virtual float CalculateDamageByContext(const FGunnerDamageContext& DamageContext) const;

public:
	UPROPERTY(EditAnywhere)
	float BackDamageMultiplier;
};
