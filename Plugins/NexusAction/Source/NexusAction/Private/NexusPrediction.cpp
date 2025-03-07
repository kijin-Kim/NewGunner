// Fill out your copyright notice in the Description page of Project Settings.


#include "NexusPrediction.h"

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
		bSerializingOwningConnection = Handle != INDEX_NONE ? ConnectionIdentifier == nullptr || Map == ConnectionIdentifier || bIsServerCreated : false;
	}
	
	Ar << bSerializingOwningConnection;
	Ar << bIsServerCreated;
	
	if (bSerializingOwningConnection)
	{
		Ar << Handle;
	}
	
	if (!bIsServerCreated)
	{
		ConnectionIdentifier = Map;
	}

	return true;
}

void FNexusPredictionTag::PostReplicatedAdd(const FNexusPredictionTagContainer& InArray)
{
	UE_LOG(LogNexus, Verbose, TEXT("PredictionTag(%d) 추가"), Handle);
	FNexusPredictionEvents::BroadcastOnPredictionEnded(*this);
}

void FNexusPredictionTag::PreReplicatedRemove(const FNexusPredictionTagContainer& InArray)
{
	UE_LOG(LogNexus, Verbose, TEXT("PredictionTag(%d) 제거"), Handle);
	FNexusPredictionEvents::BroadcastOnPredictionEnded(*this);
}

void FNexusPredictionTag::PostReplicatedChange(const FNexusPredictionTagContainer& InArray)
{
	UE_LOG(LogNexus, Verbose, TEXT("PredictionTag(%d) 변경"), Handle);
	FNexusPredictionEvents::BroadcastOnPredictionEnded(*this);
}

FNexusPredictionTagContainer::FNexusPredictionTagContainer()
{
	Items.SetNum(MaximumPredictionTags);
	for (FNexusPredictionTag& Item : Items)
	{
		MarkItemDirty(Item);
	}
}

bool FNexusPredictionTagContainer::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FNexusPredictionTag, FNexusPredictionTagContainer>(Items, DeltaParms, *this);
}

void FNexusPredictionTagContainer::ReplicatedNetPredictionTag(const FNexusPredictionTag& PredictionTag)
{
	check(PredictionTag.IsValid())
	Items[StartIndex] = PredictionTag;
	MarkItemDirty(Items[StartIndex]);
	StartIndex = (StartIndex + 1) % MaximumPredictionTags;
}

void FNexusPredictionEvents::ResetPredictionEvents()
{
	PredictionEvents.Empty();
}

void FNexusPredictionEvents::BroadcastOnPredictionEnded(const FNexusPredictionTag& PredictionTag)
{
	if (!PredictionTag.IsValid())
	{
		return;
	}

	if (FPredictionEvent* Event = PredictionEvents.Find(PredictionTag))
	{
		Event->OnPredictionEnded.Broadcast();
	}
	PredictionEvents.Remove(PredictionTag);

	for (auto It = PredictionEvents.CreateIterator(); It; ++It)
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

	if (FPredictionEvent* Event = PredictionEvents.Find(PredictionTag))
	{
		Event->OnPredictionFailed.Broadcast();
	}
	PredictionEvents.Remove(PredictionTag);
}

void FNexusPredictionEvents::Clear()
{
	PredictionEvents.Empty();
}
