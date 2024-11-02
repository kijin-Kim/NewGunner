// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerActionNetPrediction.generated.h"


class UGunnerAction;
DECLARE_MULTICAST_DELEGATE(FGunnerActionPredictionEventSiganture)

/**
 * 
 */
USTRUCT()
struct FGunnerActionNetPredictionHandle
{
	GENERATED_BODY()

	FGunnerActionNetPredictionHandle()
		: Handle(INDEX_NONE)
	{
	}

	void GenerateNewHandle();
	bool IsValid() const { return Handle != INDEX_NONE; }
	auto operator<=>(const FGunnerActionNetPredictionHandle& Other) const = default;
	

	
	FString ToString() const { return FString::Printf(TEXT("%d"), Handle); }

	friend uint32 GetTypeHash(const FGunnerActionNetPredictionHandle& DefHandle) { return ::GetTypeHash(DefHandle.Handle); }

private:
	UPROPERTY()
	int32 Handle;
};


USTRUCT()
struct FGunnerActionNetPrediction
{

	GENERATED_BODY()
	FGunnerActionNetPrediction();
	
	
public:
	FGunnerActionPredictionEventSiganture OnPredictionEnded;
	FGunnerActionPredictionEventSiganture OnPredictionFailed;
	UPROPERTY()
	FGunnerActionNetPredictionHandle Handle;
};