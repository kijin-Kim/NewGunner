// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GunnerDamageType.generated.h"


USTRUCT(BlueprintType)
struct GUNNER_API FDamageContext
{
	GENERATED_BODY()

public:
	FDamageContext()
		: Instigator(nullptr),
		  Target(nullptr),
		  HitNormal(FVector::ZeroVector),
		  HitBone(NAME_None),
		  Distance(0.0f),
		  bIsAlt(false)
	{
	}

	bool IsValid() const
	{
		return Instigator && Target;
	}

public:
	UPROPERTY()
	TObjectPtr<AController> Instigator;
	UPROPERTY()
	TObjectPtr<AActor> Target;
	FVector HitNormal;
	FName HitBone;
	float Distance;
	bool bIsAlt;
};


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

	virtual float CalculateDamageByContext(const FDamageContext& DamageContext) const;

public:
	UPROPERTY(EditAnywhere, meta = (DisplayPriority = 0))
	int32 BaseDamage;
	UPROPERTY(EditAnywhere, meta = (DisplayPriority = 1))
	float AltDamageMultiplier;
};
