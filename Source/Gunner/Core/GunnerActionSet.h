// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "GunnerActionSet.generated.h"

class UGunnerActionComponent;
class UGunnerAction;

USTRUCT()
struct FGunnerActionTagMapping
{
	GENERATED_BODY()
	
public:
	TSubclassOf<UGunnerAction> ActionClass;
	FGameplayTag ActionTag;
};


/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerActionSet : public UDataAsset
{
	GENERATED_BODY()
public:
	void BindActions(UGunnerActionComponent* ActionComponent) const;
	
private:
	UPROPERTY(EditDefaultsOnly, meta=(TitleProperty="ActionClass"))
	TArray<FGunnerActionTagMapping> ActionTagMappings;
};
