// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Action/NexusActionDefHandle.h"
#include "Prediction/NexusPrediction.h"
#include "NexusDataReplication.generated.h"


/**
 * 
 */
USTRUCT()
struct NEXUSACTION_API FNexusRepDataKey
{
	GENERATED_USTRUCT_BODY()

	FNexusRepDataKey()
	{
		
	}

	FNexusRepDataKey(const FNexusActionDefHandle& InActionDefHandle, const FNexusPredictionTag& InPrimaryPredictionTag)
		: ActionDefHandle(InActionDefHandle)
		  , PrimaryPredictionTag(InPrimaryPredictionTag)
	{
	}

	bool operator==(const FNexusRepDataKey& Other) const
	{
		return ActionDefHandle == Other.ActionDefHandle && PrimaryPredictionTag == Other.PrimaryPredictionTag;
	}

	bool operator!=(const FNexusRepDataKey& Other) const
	{
		return !(*this == Other);
	}

	friend uint32 GetTypeHash(const FNexusRepDataKey& RepDataKey)
	{
		return HashCombine(GetTypeHash(RepDataKey.ActionDefHandle), GetTypeHash(RepDataKey.PrimaryPredictionTag));
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("NexusRepDataKey={Hash=%d, %s, Primary%s}"), GetTypeHash(this), *ActionDefHandle.ToString(), *PrimaryPredictionTag.ToString());
	}

	UPROPERTY()
	FNexusActionDefHandle ActionDefHandle;

	UPROPERTY()
	FNexusPredictionTag PrimaryPredictionTag;
};


