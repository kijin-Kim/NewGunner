// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GunnerActionDefinitionHandle.generated.h"

USTRUCT()
struct FGunnerActionDefinitionHandle
{
	GENERATED_BODY()

	FGunnerActionDefinitionHandle()
		: Handle(INDEX_NONE)
	{
	}

	void GenerateNewHandle();
	bool IsValid() const { return Handle != INDEX_NONE; }
	bool operator==(const FGunnerActionDefinitionHandle& Other) const { return Handle == Other.Handle; }
	bool operator!=(const FGunnerActionDefinitionHandle& Other) const { return Handle != Other.Handle; }
	FString ToString() const { return FString::Printf(TEXT("%d"), Handle); }

	friend uint32 GetTypeHash(const FGunnerActionDefinitionHandle& SpecHandle) { return ::GetTypeHash(SpecHandle.Handle); }

private:
	UPROPERTY()
	int32 Handle;
};

