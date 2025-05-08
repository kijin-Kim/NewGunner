// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "GunnerActionSet.generated.h"

class UGunnerItemDef;
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
	TArray<TSubclassOf<UNexusAction>> ActionClasses;
	UPROPERTY(EditAnywhere)
	TArray<UGunnerItemDef*> ItemDefinitions;
};
