// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NexusActionDefHandle.generated.h"

USTRUCT()
struct NEXUSACTION_API FNexusActionDefHandle
{
	GENERATED_BODY()

	FNexusActionDefHandle()
		: Handle(INDEX_NONE)
	{
	}

	void GenerateNewHandle();
	bool IsValid() const { return Handle != INDEX_NONE; }
	bool operator==(const FNexusActionDefHandle& Other) const = default;
	FString ToString() const { return FString::Printf(TEXT("%d"), Handle); }

	friend uint32 GetTypeHash(const FNexusActionDefHandle& DefHandle) { return ::GetTypeHash(DefHandle.Handle); }

private:
	UPROPERTY()
	int32 Handle;
};

