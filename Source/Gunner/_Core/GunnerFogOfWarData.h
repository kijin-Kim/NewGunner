// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GunnerFogOfWarData.generated.h"

class UGunnerMapGeometryData;
/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerFogOfWarData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UGunnerMapGeometryData> GeometryAsset;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UTexture2D> FogTexture;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UTexture2D> RevealedTexture;
	
};
