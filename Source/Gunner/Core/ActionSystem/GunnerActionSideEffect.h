// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Gunner/Core/GunnerActionProperty.h"
#include "UObject/Object.h"
#include "GunnerActionSideEffect.generated.h"




USTRUCT()
struct FGunnerActionPropertySideEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FGameplayTag TargetProperyTag;
	UPROPERTY(EditAnywhere)
	TArray<FGunnerActionPropertyOperation> PropertyOperations;
};



/**
 * 
 */
UCLASS(Blueprintable)
class GUNNER_API UGunnerActionSideEffect : public UObject
{
	GENERATED_BODY()

	
public:
	UPROPERTY(EditAnywhere, meta = (TitleProperty = "TargetProperyTag"))
	TArray<FGunnerActionPropertySideEffect> PropertySideEffects;
	
};
