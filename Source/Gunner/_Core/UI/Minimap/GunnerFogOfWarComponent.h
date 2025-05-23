// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GunnerFogOfWarComponent.generated.h"


class UGunnerMapGeometryData;
class UCanvasRenderTarget2D;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UGunnerFogOfWarComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGunnerFogOfWarComponent();
	void SetupFogOfWar(APlayerState* PlayerState);

private:
	UFUNCTION()
	void DrawVision(UCanvas* Canvas, int32 Width, int32 Height);


protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UGunnerMapGeometryData> GeometryAsset;
	
};
