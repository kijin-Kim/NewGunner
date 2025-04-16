// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NexusSideEffectInstanceHandle.generated.h"

/**
 * 
 */
USTRUCT()
struct FNexusSideEffectInstanceHandle
{
	GENERATED_BODY()

	FNexusSideEffectInstanceHandle()
		: Handle(INDEX_NONE)
	{
	}

	void GenerateNewHandle();
	bool IsValid() const { return Handle != INDEX_NONE; }
	bool operator==(const FNexusSideEffectInstanceHandle& Other) const = default;
	FString ToString() const { return FString::Printf(TEXT("%d"), Handle); }

	friend uint32 GetTypeHash(const FNexusSideEffectInstanceHandle& DefHandle) { return GetTypeHash(DefHandle.Handle); }

private:
	UPROPERTY()
	int32 Handle;
};

