// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NexusEventManagerComponent.h"
#include "UObject/Interface.h"
#include "NexusEventInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UNexusEventInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

/**
 * 
 */
class NEXUSACTION_API INexusEventInterface
{
	GENERATED_IINTERFACE_BODY()

protected:
	void BindEvents();
	void UnbindEvents(UNexusEventManagerComponent* EventManagerComponent);
	virtual TArray<FNexusEventCallbackHandle> SetupEvents() = 0;

private:
	TArray<FNexusEventCallbackHandle> BoundedEventCallbackHandles;
};
