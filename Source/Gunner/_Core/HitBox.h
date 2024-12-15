// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitBox.generated.h"


USTRUCT()
struct FHitBox
{
	GENERATED_BODY()

public:
	void DrawDebug(const UWorld* InWorld, FColor Color, bool bPersistentLines, float LifeTime = -1.0f) const
	{
		DrawDebugCapsule(InWorld, Transform.GetLocation(), HalfHeight, Radius, Transform.GetRotation(), Color, bPersistentLines, LifeTime);
	}

public:
	UPROPERTY()
	FTransform Transform;
	UPROPERTY()
	float HalfHeight;
	UPROPERTY()
	float Radius;
	UPROPERTY()
	FName BoneName;
};
