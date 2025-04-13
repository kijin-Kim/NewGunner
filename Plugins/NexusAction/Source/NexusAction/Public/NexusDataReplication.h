// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Action/NexusActionDefHandle.h"
#include "NexusPrediction.h"
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

	UPROPERTY()
	FNexusActionDefHandle ActionDefHandle;

	UPROPERTY()
	FNexusPredictionTag PrimaryPredictionTag;
};


