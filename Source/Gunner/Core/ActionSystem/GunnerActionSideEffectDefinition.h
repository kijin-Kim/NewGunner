// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerActionSideEffectDefinitionHandle.h"
#include "GunnerActionSideEffectDefinition.generated.h"

class UGunnerActionSideEffect;
/**
 * 
 */
USTRUCT()
struct FGunnerActionSideEffectDefinition
{
	GENERATED_BODY()

	FGunnerActionSideEffectDefinition();
	FGunnerActionSideEffectDefinition(TSubclassOf<UGunnerActionSideEffect> InActionClass);
	bool operator==(const FGunnerActionSideEffectDefinition& Other) const;
	bool operator!=(const FGunnerActionSideEffectDefinition& Other) const;

	UPROPERTY()
	FGunnerActionSideEffectDefinitionHandle Handle;
	UPROPERTY()
	TSubclassOf<UGunnerActionSideEffect> SideEffectClass;

	UPROPERTY()
	TObjectPtr<UGunnerActionSideEffect> SideEffectCDO;
};

