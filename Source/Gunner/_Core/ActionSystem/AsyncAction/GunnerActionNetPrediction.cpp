// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionNetPrediction.h"

#include "Gunner/Gunner.h"

void FGunnerActionNetPredictionHandle::GenerateNewHandle()
{
	static int32 HandleCounter = 1;
	Handle = HandleCounter++;
	bIsExpired = false;
}

void FGunnerActionNetPredictionHandle::Expire()
{
	bIsExpired = true;
}

void FGunnerActionNetPredictionHandle::PostReplicatedAdd(const FGunnerActionNetPredictionHandleArray& InArray)
{
	Expire();
	FGunneractionNetPredictionEvents::BroadcastOnPredictionEnded(*this);
}

void FGunnerActionNetPredictionHandle::PreReplicatedRemove(const FGunnerActionNetPredictionHandleArray& InArray)
{
	Expire();
	FGunneractionNetPredictionEvents::BroadcastOnPredictionEnded(*this);
}

void FGunnerActionNetPredictionHandle::PostReplicatedChange(const FGunnerActionNetPredictionHandleArray& InArray)
{
	Expire();
	FGunneractionNetPredictionEvents::BroadcastOnPredictionEnded(*this);
}

FGunnerActionNetPredictionHandleArray::FGunnerActionNetPredictionHandleArray()
{
	Items.SetNum(MaximumPredictionHandles);
	for (FGunnerActionNetPredictionHandle& Item : Items)
	{
		MarkItemDirty(Item);
	}
}

bool FGunnerActionNetPredictionHandleArray::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FGunnerActionNetPredictionHandle, FGunnerActionNetPredictionHandleArray>(Items, DeltaParms, *this);
}

void FGunnerActionNetPredictionHandleArray::ReplicatedNetPredictionHandle(const FGunnerActionNetPredictionHandle& PredictionHandle)
{
	if (PredictionHandle.IsExpired())
	{
		return;
	}

	Items[StartIndex] = PredictionHandle;
	MarkItemDirty(Items[StartIndex]);
	StartIndex = (StartIndex + 1) % MaximumPredictionHandles;
}

void FGunneractionNetPredictionEvents::ResetPredictionEvents()
{
	PredictionEvents.Empty();
}

void FGunneractionNetPredictionEvents::BroadcastOnPredictionEnded(const FGunnerActionNetPredictionHandle& PredictionHandle)
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

void FGunneractionNetPredictionEvents::BroadcastOnPredictionFailed(const FGunnerActionNetPredictionHandle& PredictionHandle)
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
