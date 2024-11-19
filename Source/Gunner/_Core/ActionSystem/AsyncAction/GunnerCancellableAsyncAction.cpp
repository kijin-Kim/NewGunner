// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerCancellableAsyncAction.h"

bool UGunnerCancellableAsyncAction::ShouldBroadcastDelegates() const
{
	if (!Action.IsValid())
	{
		return false;
	}
	return Super::ShouldBroadcastDelegates();
}

void UGunnerCancellableAsyncAction::Cancel()
{
	if (RegisteredWithGameInstance.IsValid())
	{
		SetReadyToDestroy();
	}
}
