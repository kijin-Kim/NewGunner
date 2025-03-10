// Fill out your copyright notice in the Description page of Project Settings.


#include "Cue/NexusCue.h"


UWorld* UNexusCue::GetWorld() const
{
	// https://forums.unrealengine.com/t/can-you-use-a-blueprint-function-library-in-an-object-class/350918/37
	if (HasAllFlags(RF_ClassDefaultObject))
	{
		return nullptr;
	}
	return GetOuter()->GetWorld();
}

void UNexusCue::CallOnTriggered(const FNexusTargetDataHandle& InTargetDataHandle)
{
	TargetDataHandle = InTargetDataHandle;
	OnTriggered(InTargetDataHandle);
	BP_OnTriggered();
	TargetDataHandle = FNexusTargetDataHandle();
}

void UNexusCue::OnTriggered(const FNexusTargetDataHandle& InTargetDataHandle)
{
}
