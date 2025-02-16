// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "NexusPrediction.generated.h"


class UNexusAction;
struct FNexusPredictionTagContainer;

DECLARE_MULTICAST_DELEGATE(FNexusPredictionEventSignature)

/**
 * 
 */
USTRUCT()
struct NEXUSACTION_API FNexusPredictionTag  : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FNexusPredictionTag()
		: Handle(INDEX_NONE)
	{
	}

	auto operator<=>(const FNexusPredictionTag& Other) const
	{
		return Handle <=> Other.Handle;
	}

	bool operator==(const FNexusPredictionTag& Other) const
	{
		return Handle == Other.Handle;
	}
	
	
	//~ Begin FFastArraySerializerItem Interface.
	void PostReplicatedAdd(const FNexusPredictionTagContainer& InArray);
	void PreReplicatedRemove(const FNexusPredictionTagContainer& InArray);
	void PostReplicatedChange(const FNexusPredictionTagContainer& InArray);
	//~ End FFastArraySerializerItem Interface.

	void OnRepPredictionHandle();

	void GenerateNewHandle();
	void Expire();
	bool IsValid() const { return Handle != INDEX_NONE; }
	bool IsExpired() const { return IsValid() && bIsExpired; }


	FString ToString() const { return FString::Printf(TEXT("%d"), Handle); }
	friend uint32 GetTypeHash(const FNexusPredictionTag& DefHandle) { return ::GetTypeHash(DefHandle.Handle); }

private:
	UPROPERTY()
	int32 Handle;
	bool bIsExpired = false;
};


USTRUCT()
struct NEXUSACTION_API FNexusPredictionTagContainer : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	FNexusPredictionTagContainer();

	//~ Begin FFastArraySerializer Interface.
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);
	//~ End FFastArraySerializer Interface.

	void ReplicatedNetPredictionHandle(const FNexusPredictionTag& PredictionHandle);

	UPROPERTY()
	TArray<FNexusPredictionTag> Items;

	int32 StartIndex = 0;
	static constexpr int32 MaximumPredictionHandles = 16;
};

template <>
struct TStructOpsTypeTraits<FNexusPredictionTagContainer> : public TStructOpsTypeTraitsBase2<FNexusPredictionTagContainer>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};


class NEXUSACTION_API FNexusPredictionEvents
{
public:
	struct FPredictionEvent
	{
		FNexusPredictionEventSignature OnPredictionEnded;
		FNexusPredictionEventSignature OnPredictionFailed;
	};

	static void ResetPredictionEvents();
	static void BroadcastOnPredictionEnded(const FNexusPredictionTag& PredictionHandle);
	static void BroadcastOnPredictionFailed(const FNexusPredictionTag& PredictionHandle);
	static FPredictionEvent& GetPredictionEvent(const FNexusPredictionTag& PredictionHandle) { return PredictionEvents.FindOrAdd(PredictionHandle); }
	static void Clear();

private:
	inline static TMap<FNexusPredictionTag, FPredictionEvent> PredictionEvents;
};
