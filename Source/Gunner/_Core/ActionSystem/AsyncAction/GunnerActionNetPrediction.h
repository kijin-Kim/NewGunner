// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "GunnerActionNetPrediction.generated.h"


class UGunnerAction;
struct FGunnerActionNetPredictionHandleArray;

DECLARE_MULTICAST_DELEGATE(FGunnerActionPredictionEventSiganture)

/**
 * 
 */
USTRUCT()
struct FGunnerActionNetPredictionHandle  : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FGunnerActionNetPredictionHandle()
		: Handle(INDEX_NONE)
	{
	}

	auto operator<=>(const FGunnerActionNetPredictionHandle& Other) const
	{
		return Handle <=> Other.Handle;
	}

	bool operator==(const FGunnerActionNetPredictionHandle& Other) const
	{
		return Handle == Other.Handle;
	}
	
	
	//~ Begin FFastArraySerializerItem Interface.
	void PostReplicatedAdd(const FGunnerActionNetPredictionHandleArray& InArray);
	void PreReplicatedRemove(const FGunnerActionNetPredictionHandleArray& InArray);
	void PostReplicatedChange(const FGunnerActionNetPredictionHandleArray& InArray);
	//~ End FFastArraySerializerItem Interface.

	void OnRepPredictionHandle();

	void GenerateNewHandle();
	void Expire();
	bool IsValid() const { return Handle != INDEX_NONE; }
	bool IsExpired() const { return IsValid() && bIsExpired; }


	FString ToString() const { return FString::Printf(TEXT("%d"), Handle); }
	friend uint32 GetTypeHash(const FGunnerActionNetPredictionHandle& DefHandle) { return ::GetTypeHash(DefHandle.Handle); }

private:
	UPROPERTY()
	int32 Handle;
	bool bIsExpired = false;
};


USTRUCT()
struct FGunnerActionNetPredictionHandleArray : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	FGunnerActionNetPredictionHandleArray();

	//~ Begin FFastArraySerializer Interface.
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);
	//~ End FFastArraySerializer Interface.

	void ReplicatedNetPredictionHandle(FGunnerActionNetPredictionHandle PredictionHandle);

	UPROPERTY()
	TArray<FGunnerActionNetPredictionHandle> Items;

	int32 StartIndex = 0;
	static constexpr int32 MaximumPredictionHandles = 16;
};

template <>
struct TStructOpsTypeTraits<FGunnerActionNetPredictionHandleArray> : public TStructOpsTypeTraitsBase2<FGunnerActionNetPredictionHandleArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};


class FGunneractionNetPredictionEvents
{
public:
	struct FPredictionEvent
	{
		FGunnerActionPredictionEventSiganture OnPredictionEnded;
		FGunnerActionPredictionEventSiganture OnPredictionFailed;
	};

	static void ResetPredictionEvents();
	static void BroadcastOnPredictionEnded(FGunnerActionNetPredictionHandle PredictionHandle);
	static void BroadcastOnPredictionFailed(FGunnerActionNetPredictionHandle PredictionHandle);

	static FPredictionEvent& GetPredictionEvent(FGunnerActionNetPredictionHandle PredictionHandle) { return PredictionEvents.FindOrAdd(PredictionHandle); }

private:
	inline static TMap<FGunnerActionNetPredictionHandle, FPredictionEvent> PredictionEvents;
};
