// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NexusSideEffectDefHandle.generated.h"

/**
 * 
 */
USTRUCT()
struct FNexusSideEffectDefHandle
{
	GENERATED_BODY()

	FNexusSideEffectDefHandle()
		: Handle(INDEX_NONE)
	{
	}

	void GenerateNewHandle();
	bool IsValid() const { return Handle != INDEX_NONE; }
	bool operator==(const FNexusSideEffectDefHandle& Other) const = default;
	FString ToString() const { return FString::Printf(TEXT("%d"), Handle); }

	friend uint32 GetTypeHash(const FNexusSideEffectDefHandle& DefHandle) { return GetTypeHash(DefHandle.Handle); }

private:
	UPROPERTY()
	int32 Handle;
};

