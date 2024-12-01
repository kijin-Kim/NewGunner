// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionSign.h"

void UGunnerActionSign::OnSignaled_Implementation()
{
}

UWorld* UGunnerActionSign::GetWorld() const
{
	// https://forums.unrealengine.com/t/can-you-use-a-blueprint-function-library-in-an-object-class/350918/37
	if (HasAllFlags(RF_ClassDefaultObject))
	{
		return nullptr;
	}
	return GetOuter()->GetWorld();
}