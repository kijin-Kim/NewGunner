// Fill out your copyright notice in the Description page of Project Settings.


#include "NexusPrediction.h"

void FNexusPredictionTag::GenerateNewHandle()
{
	static int32 HandleCounter = 1;
	Handle = HandleCounter++;
	bIsExpired = false;
}

void FNexusPredictionTag::Expire()
{
	bIsExpired = true;
}

void FNexusPredictionTag::PostReplicatedAdd(const FNexusPredictionTagContainer& InArray)
{
	Expire();
	FNexusPredictionEvents::BroadcastOnPredictionEnded(*this);
}

void FNexusPredictionTag::PreReplicatedRemove(const FNexusPredictionTagContainer& InArray)
{
	Expire();
	FNexusPredictionEvents::BroadcastOnPredictionEnded(*this);
}

void FNexusPredictionTag::PostReplicatedChange(const FNexusPredictionTagContainer& InArray)
{
	Expire();
	FNexusPredictionEvents::BroadcastOnPredictionEnded(*this);
}

FNexusPredictionTagContainer::FNexusPredictionTagContainer()
{
	Items.SetNum(MaximumPredictionHandles);
	for (FNexusPredictionTag& Item : Items)
	{
		MarkItemDirty(Item);
	}
}

bool FNexusPredictionTagContainer::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FNexusPredictionTag, FNexusPredictionTagContainer>(Items, DeltaParms, *this);
}

void FNexusPredictionTagContainer::ReplicatedNetPredictionHandle(const FNexusPredictionTag& PredictionHandle)
{
	if (PredictionHandle.IsExpired())
	{
		return;
	}

	Items[StartIndex] = PredictionHandle;
	MarkItemDirty(Items[StartIndex]);
	StartIndex = (StartIndex + 1) % MaximumPredictionHandles;
}

void FNexusPredictionEvents::ResetPredictionEvents()
{
	PredictionEvents.Empty();
}

void FNexusPredictionEvents::BroadcastOnPredictionEnded(const FNexusPredictionTag& PredictionHandle)
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

void FNexusPredictionEvents::BroadcastOnPredictionFailed(const FNexusPredictionTag& PredictionHandle)
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

void FNexusPredictionEvents::Clear()
{
	PredictionEvents.Empty();
}
