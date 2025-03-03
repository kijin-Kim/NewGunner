// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GunnerDamageType.generated.h"

class UGunnerDamageContext;
/**
 * 
 */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, CollapseCategories, Abstract)
class GUNNER_API UGunnerDamageType : public UObject
{
	GENERATED_BODY()

public:
	UGunnerDamageType()
		: BaseDamage(0.0f),
		  AltDamageMultiplier(1.0f)
	{
	}

	virtual float CalculateDamageByContext(UGunnerDamageContext* DamageContext) const;

public:
	UPROPERTY(EditAnywhere, meta = (DisplayPriority = 0))
	int32 BaseDamage;
	UPROPERTY(EditAnywhere, meta = (DisplayPriority = 1))
	float AltDamageMultiplier;
};
