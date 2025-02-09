// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GunnerMapDataSet.generated.h"


class UGunnerMapData;
/**
 * 
 */
UCLASS(BlueprintType)
class GUNNER_API UGunnerMapDataSet : public UDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	UGunnerMapData* GetMapDataByName(FString MapName) const;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<UGunnerMapData>> TeamDeathMatchMaps;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<UGunnerMapData>> StandardAndDeathMatchMaps;
	 
};
