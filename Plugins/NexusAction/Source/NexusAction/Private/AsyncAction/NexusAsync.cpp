// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncAction/NexusAsync.h"

bool UNexusAsync::ShouldBroadcastDelegates() const
{
	if (!Action.IsValid())
	{
		return false;
	}
	return Super::ShouldBroadcastDelegates();
}

void UNexusAsync::Cancel()
{
	if (RegisteredWithGameInstance.IsValid())
	{
		SetReadyToDestroy();
	}
}
