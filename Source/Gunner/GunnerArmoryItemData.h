// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GunnerArmoryItemData.generated.h"


class UGunnerEquipmentDef;

/**
 * 
 */
UCLASS(BlueprintType)
class GUNNER_API UGunnerArmoryItemDef : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UGunnerEquipmentDef> EquipmentDataAsset;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ArmoryCost;
	
	// TODO: Enum
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ArmoryCategoryName;
};


UCLASS(BlueprintType)
class GUNNER_API UGunnerArmoryItemDefSet : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (TitleProperty = "ArmoryItemDefinitions"))
	TArray<TObjectPtr<UGunnerArmoryItemDef>> ArmoryItemDefinitions;
};
