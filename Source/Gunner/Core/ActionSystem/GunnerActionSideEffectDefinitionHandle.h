// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerActionSideEffectDefinitionHandle.generated.h"

/**
 * 
 */
USTRUCT()
struct FGunnerActionSideEffectDefinitionHandle
{
	GENERATED_BODY()

	FGunnerActionSideEffectDefinitionHandle()
		: Handle(INDEX_NONE)
	{
	}

	void GenerateNewHandle();
	bool IsValid() const { return Handle != INDEX_NONE; }
	bool operator==(const FGunnerActionSideEffectDefinitionHandle& Other) const = default;
	FString ToString() const { return FString::Printf(TEXT("%d"), Handle); }

	friend uint32 GetTypeHash(const FGunnerActionSideEffectDefinitionHandle& DefHandle) { return ::GetTypeHash(DefHandle.Handle); }

private:
	UPROPERTY()
	int32 Handle;
};

