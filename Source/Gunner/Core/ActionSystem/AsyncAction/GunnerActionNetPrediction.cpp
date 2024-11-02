// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionNetPrediction.h"

void FGunnerActionNetPredictionHandle::GenerateNewHandle()
{
	static int32 HandleCounter = 1;
	Handle = HandleCounter++;
}

FGunnerActionNetPrediction::FGunnerActionNetPrediction()
{
	Handle.GenerateNewHandle();
}