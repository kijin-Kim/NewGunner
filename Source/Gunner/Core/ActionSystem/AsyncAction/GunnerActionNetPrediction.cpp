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