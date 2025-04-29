// Fill out your copyright notice in the Description page of Project Settings.


#include "Prediction/NexusPrediction.h"

#include "NexusLog.h"

void FNexusPredictionTag::GenerateNewHandle(bool bIsServer)
{
	static int32 HandleCounter = 1;
	Handle = HandleCounter++;
	bIsServerCreated = bIsServer;
}


bool FNexusPredictionTag::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	bool bSerializingOwningConnection = false;
	if (Ar.IsSaving())
	{
		// 서버 생성 예측태그는 모든 클라이언트에서 유효합니다.
		// 클라이언트 생성 예측태그는 해당 클라이언트와 서버에서만 유효합니다.
		// 주: 유효하다는 것은 Handle이 INDEX_NONE이 아니라는 것을 의미합니다.
		bSerializingOwningConnection = Handle != INDEX_NONE ? ConnectionIdentifier == nullptr || Map == ConnectionIdentifier || bIsServerCreated : false;
	}

	Ar << bSerializingOwningConnection;
	Ar << bIsServerCreated;

	if (bSerializingOwningConnection)
	{
		Ar << Handle;
	}

	if (Ar.IsLoading() && !bIsServerCreated)
	{
		ConnectionIdentifier = Map;
	}

	bOutSuccess = true;
	return true;
}

void FNexusPredictionTag::PostReplicatedAdd(const FNexusPredictionTagContainer& InArray)
{
	FNexusPredictionEvents::BroadcastOnPredictionEnded(*this);
}

void FNexusPredictionTag::PreReplicatedRemove(const FNexusPredictionTagContainer& InArray)
{
	FNexusPredictionEvents::BroadcastOnPredictionEnded(*this);
}

void FNexusPredictionTag::PostReplicatedChange(const FNexusPredictionTagContainer& InArray)
{
	checkNoEntry();
	FNexusPredictionEvents::BroadcastOnPredictionEnded(*this);
}


bool FNexusPredictionTagContainer::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FNexusPredictionTag, FNexusPredictionTagContainer>(Items, DeltaParms, *this);
}

void FNexusPredictionTagContainer::ReplicateNetPredictionTag(const FNexusPredictionTag& PredictionTag)
{
	check(PredictionTag.IsValid())
	Items[StartIndex] = PredictionTag;
	MarkItemDirty(Items[StartIndex]);
	StartIndex = (StartIndex + 1) % MaximumPredictionTags;
}

void FNexusPredictionEvents::ResetPredictionEvents()
{
	PredictionEventMap.Empty();
}

void FNexusPredictionEvents::BroadcastOnPredictionEnded(const FNexusPredictionTag& PredictionTag)
{
	if (!PredictionTag.IsValid())
	{
		return;
	}

	if (FPredictionEvent* Event = PredictionEventMap.Find(PredictionTag))
	{
		Event->OnPredictionEnded.Broadcast();
	}
	PredictionEventMap.Remove(PredictionTag);

	for (auto It = PredictionEventMap.CreateIterator(); It; ++It)
	{
		if (It.Key() <= PredictionTag)
		{
			It.Value().OnPredictionEnded.Broadcast();
			It.RemoveCurrent();
		}
	}
}

void FNexusPredictionEvents::BroadcastOnPredictionFailed(const FNexusPredictionTag& PredictionTag)
{
	if (!PredictionTag.IsValid())
	{
		return;
	}

	if (FPredictionEvent* Event = PredictionEventMap.Find(PredictionTag))
	{
		Event->OnPredictionFailed.Broadcast();
	}
	PredictionEventMap.Remove(PredictionTag);
}
