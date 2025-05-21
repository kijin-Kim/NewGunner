// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerMiniMapData.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FGunnerGeometryVertex
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere)
	float X = 0.0f;
	UPROPERTY(EditAnywhere)
	float Y = 0.0f;
};

USTRUCT(BlueprintType)
struct FGunnerGeometryLine
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere)
	int32 Start = 0;
	UPROPERTY(EditAnywhere)
	int32 End = 0;
};

USTRUCT(BlueprintType)
struct FGunnerEdgeSegment
{
	GENERATED_BODY();
	FVector2D From;
	FVector2D To;
	float ZHeight = 0.0f; // g.Y 값
};

USTRUCT(BlueprintType)
struct FGunnerGeometryGroup
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere)
	TArray<FGunnerGeometryVertex> Vertices;

	UPROPERTY(EditAnywhere)
	TArray<FGunnerGeometryLine> Lines;

	UPROPERTY(EditAnywhere)
	float ZHeight = 0.f; // g.Y 값
};


UCLASS(BlueprintType)
class UGunnerMapGeometryData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FGunnerGeometryGroup> Groups;
};

