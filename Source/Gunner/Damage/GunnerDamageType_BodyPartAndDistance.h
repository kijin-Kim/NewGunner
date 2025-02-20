// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gunner/_Core/Damage/GunnerDamageType.h"
#include "GunnerDamageType_BodyPartAndDistance.generated.h"

/**
 * 
 */
USTRUCT()
struct GUNNER_API FDistanceDamageFallOff
{
	GENERATED_BODY()

public:
	FDistanceDamageFallOff()
		: StartDistance(0.0f),
		  Multiplier(1.0f)
	{
	}

public:
	UPROPERTY(EditAnywhere, meta = (ForceUnits = "m"))
	int32 StartDistance;
	UPROPERTY(EditAnywhere)
	float Multiplier;
};

UCLASS(DisplayName = "BodyPart And Distance")
class GUNNER_API UGunnerDamageType_BodyPartAndDistance : public UGunnerDamageType
{
	GENERATED_BODY()

public:
	UGunnerDamageType_BodyPartAndDistance()
		: HeadDamageMultiplier(1.0f),
		  LegDamageMultiplier(1.0f)
	{
	}

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	virtual float CalculateDamageByContext(const FDamageContext& DamageContext) const override;

private:
	float GetDistanceFallOffMultiplier(const FDamageContext& DamageContext) const;
	float GetBodyPartMultiplier(const FDamageContext& DamageContext) const;

public:
	// 머리 부위 피해 배율. BaseDamage를 몸통 부위에 맞았을 때의 피해로 간주합니다.
	UPROPERTY(EditAnywhere)
	float HeadDamageMultiplier;
	// 다리 부위 피해 배율. BaseDamage를 몸통 부위에 맞았을 때의 피해로 간주합니다.
	UPROPERTY(EditAnywhere)
	float LegDamageMultiplier;
	UPROPERTY(EditAnywhere, meta = (TitleProperty = "{StartDistance}m, {Multiplier}"))
	TArray<FDistanceDamageFallOff> DistanceDamageFallOffs;
};
