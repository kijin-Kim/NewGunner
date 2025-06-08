// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "GunnerWorldSettings.generated.h"

class UGunnerFogOfWarData;
class UGunnerMapGeometryData;
/**
 * 
 */
UCLASS()
class GUNNER_API AGunnerWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:
	UGunnerFogOfWarData* GetFogOfWarData() const
	{
		return FogOfWarData;
	}

protected:
	UPROPERTY(EditAnywhere, Category = "GameMode")
	TObjectPtr<UGunnerFogOfWarData> FogOfWarData;
};
