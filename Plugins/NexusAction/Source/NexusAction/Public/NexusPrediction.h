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
struct NEXUSACTION_API FNexusPredictionTag : public FFastArraySerializerItem
{
	GENERATED_BODY()

	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess);

	auto operator<=>(const FNexusPredictionTag& Other) const
	{
		return Handle <=> Other.Handle;
	}

	bool operator==(const FNexusPredictionTag& Other) const
	{
		return Handle == Other.Handle && bIsServerCreated == Other.bIsServerCreated;
	}

	//~ Begin FFastArraySerializerItem Interface.
	void PostReplicatedAdd(const FNexusPredictionTagContainer& InArray);
	void PreReplicatedRemove(const FNexusPredictionTagContainer& InArray);
	void PostReplicatedChange(const FNexusPredictionTagContainer& InArray);
	//~ End FFastArraySerializerItem Interface.


	void GenerateNewHandle(bool bIsServer);
	bool IsValid() const { return Handle != INDEX_NONE; }
	bool IsPredictable() const { return IsValid() && !bIsServerCreated; }
	bool IsServerCreated() const { return bIsServerCreated; }


	FString ToString() const { return FString::Printf(TEXT("PredictionTag(%d, %s)"), Handle, bIsServerCreated ? TEXT("Server") : TEXT("Client")); }
	friend uint32 GetTypeHash(const FNexusPredictionTag& DefHandle) { return ::GetTypeHash(DefHandle.Handle); }

private:
	UPROPERTY()
	int32 Handle = INDEX_NONE;
	UPROPERTY()
	bool bIsServerCreated = false;

	// 서버에서 각 커넥션에 대한 UPackage
	UPackageMap* ConnectionIdentifier = nullptr;
};

template <>
struct TStructOpsTypeTraits<FNexusPredictionTag> : public TStructOpsTypeTraitsBase2<FNexusPredictionTag>
{
	enum
	{
		WithNetSerializer = true,
		WithIdenticalViaEquality = true
	};
};
 


USTRUCT()
struct NEXUSACTION_API FNexusPredictionTagContainer : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	FNexusPredictionTagContainer();

	//~ Begin FFastArraySerializer Interface.
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);
	//~ End FFastArraySerializer Interface.

	void ReplicatedNetPredictionTag(const FNexusPredictionTag& PredictionTag);

	UPROPERTY()
	TArray<FNexusPredictionTag> Items;

	int32 StartIndex = 0;
	static constexpr int32 MaximumPredictionTags = 16;
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
	static void BroadcastOnPredictionEnded(const FNexusPredictionTag& PredictionTag);
	static void BroadcastOnPredictionFailed(const FNexusPredictionTag& PredictionTag);
	static FPredictionEvent& GetPredictionEvent(const FNexusPredictionTag& PredictionTag) { return PredictionEvents.FindOrAdd(PredictionTag); }
	static void Clear();

private:
	inline static TMap<FNexusPredictionTag, FPredictionEvent> PredictionEvents;
};
