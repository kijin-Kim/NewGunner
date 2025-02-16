// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "GunnerActionSet.generated.h"

class UNexusAction;
class UNexusSideEffect;
/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerActionSet : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TMap<FGameplayTag, float> InitialProperties;
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<UNexusAction>> InitialActionClasses;
};
