// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"
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

	auto operator<=>(const FGunnerActionNetPredictionHandle& Other) const
	{
		return Handle <=> Other.Handle;
	}
	
	bool operator==(const FGunnerActionNetPredictionHandle& Other) const
	{
		return Handle == Other.Handle;
	}

	void GenerateNewHandle();
	void Expire() { bIsExpired = true; }
	bool IsValid() const { return Handle != INDEX_NONE; }
	bool IsExpired() const { return bIsExpired; }


	FString ToString() const { return FString::Printf(TEXT("%d"), Handle); }

	friend uint32 GetTypeHash(const FGunnerActionNetPredictionHandle& DefHandle) { return ::GetTypeHash(DefHandle.Handle); }

private:
	UPROPERTY()
	int32 Handle;
	bool bIsExpired = false;
};

struct FGunnerActionNetPredictionHandleArray;

USTRUCT()
struct FGunnerActionNetPredictionHandleItem : public FFastArraySerializerItem
{
	GENERATED_USTRUCT_BODY()

public:
	void PostReplicatedAdd(const FGunnerActionNetPredictionHandleArray& InArray);
	void PreReplicatedRemove(const FGunnerActionNetPredictionHandleArray& InArray);
	void PostReplicatedChange(const FGunnerActionNetPredictionHandleArray& InArray);
	void OnRepPredictionHandle();

public:
	UPROPERTY()
	FGunnerActionNetPredictionHandle PredictionHandle;
};

USTRUCT()
struct FGunnerActionNetPredictionHandleArray : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	FGunnerActionNetPredictionHandleArray()
	{
		Items.SetNum(MaximumPredictionHandles);
		for (FGunnerActionNetPredictionHandleItem& Item : Items)
		{
			MarkItemDirty(Item);
		}
	}

	void ReplicatedNetPredictionHandle(FGunnerActionNetPredictionHandle PredictionHandle)
	{
		if (PredictionHandle.IsExpired())
		{
			return;
		}

		Items[StartIndex].PredictionHandle = PredictionHandle;
		MarkItemDirty(Items[StartIndex]);
		StartIndex = (StartIndex + 1) % MaximumPredictionHandles;
	}

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FGunnerActionNetPredictionHandleItem, FGunnerActionNetPredictionHandleArray>(Items, DeltaParms, *this);
	}

	UPROPERTY()
	TArray<FGunnerActionNetPredictionHandleItem> Items;

	int32 StartIndex = 0;
	inline static const int32 MaximumPredictionHandles = 16;
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

	static void ResetPredictionEvents()
	{
		PredictionEvents.Empty();
	}

	static FPredictionEvent& GetPredictionEvent(FGunnerActionNetPredictionHandle PredictionHandle)
	{
		return PredictionEvents.FindOrAdd(PredictionHandle);
	}

	static void BroadcastOnPredictionEnded(FGunnerActionNetPredictionHandle PredictionHandle)
	{
		if (!PredictionHandle.IsValid())
		{
			return;
		}

		if (FPredictionEvent* Event = PredictionEvents.Find(PredictionHandle))
		{
			Event->OnPredictionEnded.Broadcast();
		}
		PredictionEvents.Remove(PredictionHandle);

		for (auto It = PredictionEvents.CreateIterator(); It; ++It)
		{
			if (It.Key() <= PredictionHandle)
			{
				It.Value().OnPredictionEnded.Broadcast();
				It.RemoveCurrent();
			}
		}
	}

	static void BroadcastOnPredictionFailed(FGunnerActionNetPredictionHandle PredictionHandle)
	{
		if (!PredictionHandle.IsValid())
		{
			return;
		}

		if (FPredictionEvent* Event = PredictionEvents.Find(PredictionHandle))
		{
			Event->OnPredictionFailed.Broadcast();
		}
		PredictionEvents.Remove(PredictionHandle);
	}

private:
	inline static TMap<FGunnerActionNetPredictionHandle, FPredictionEvent> PredictionEvents;
};



