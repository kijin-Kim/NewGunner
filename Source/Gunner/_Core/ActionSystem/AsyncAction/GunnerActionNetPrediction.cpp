// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionNetPrediction.h"

void FGunnerActionNetPredictionHandle::GenerateNewHandle()
{
	static int32 HandleCounter = 1;
	Handle = HandleCounter++;
	bIsExpired = false;
}

void FGunnerActionNetPredictionHandleItem::PostReplicatedAdd(const FGunnerActionNetPredictionHandleArray& InArray)
{
	PredictionHandle.Expire();
	FGunneractionNetPredictionEvents::BroadcastOnPredictionEnded(PredictionHandle);
}

void FGunnerActionNetPredictionHandleItem::PreReplicatedRemove(const FGunnerActionNetPredictionHandleArray& InArray)
{
	PredictionHandle.Expire();
	FGunneractionNetPredictionEvents::BroadcastOnPredictionEnded(PredictionHandle);
}

void FGunnerActionNetPredictionHandleItem::PostReplicatedChange(const FGunnerActionNetPredictionHandleArray& InArray)
{
	PredictionHandle.Expire();
	FGunneractionNetPredictionEvents::BroadcastOnPredictionEnded(PredictionHandle);
}

FGunnerActionNetPredictionHandleArray::FGunnerActionNetPredictionHandleArray()
{
	Items.SetNum(MaximumPredictionHandles);
	for (FGunnerActionNetPredictionHandleItem& Item : Items)
	{
		MarkItemDirty(Item);
	}
}

bool FGunnerActionNetPredictionHandleArray::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FGunnerActionNetPredictionHandleItem, FGunnerActionNetPredictionHandleArray>(Items, DeltaParms, *this);
}

void FGunnerActionNetPredictionHandleArray::ReplicatedNetPredictionHandle(FGunnerActionNetPredictionHandle PredictionHandle)
{
	if (PredictionHandle.IsExpired())
	{
		return;
	}

	Items[StartIndex].PredictionHandle = PredictionHandle;
	MarkItemDirty(Items[StartIndex]);
	StartIndex = (StartIndex + 1) % MaximumPredictionHandles;
}

void FGunneractionNetPredictionEvents::ResetPredictionEvents()
{
	PredictionEvents.Empty();
}

void FGunneractionNetPredictionEvents::BroadcastOnPredictionEnded(FGunnerActionNetPredictionHandle PredictionHandle)
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

void FGunneractionNetPredictionEvents::BroadcastOnPredictionFailed(FGunnerActionNetPredictionHandle PredictionHandle)
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

