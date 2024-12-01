// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerEventManagerComponent.h"
#include "UObject/Interface.h"
#include "GunnerEventCallbackBindInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UGunnerEventCallbackBindInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GUNNER_API IGunnerEventCallbackBindInterface
{
	GENERATED_BODY()

protected:
	void BindEvents();
	void UnbindEvents(UGunnerEventManagerComponent* EventManagerComponent);
	virtual TArray<FGunnerEventCallbackHandle> SetupEvents() = 0;

private:
	TArray<FGunnerEventCallbackHandle> BoundedEventCallbackHandles;
};
