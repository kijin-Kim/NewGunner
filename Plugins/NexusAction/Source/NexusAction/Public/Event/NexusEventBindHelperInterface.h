// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NexusEventManagerComponent.h"
#include "UObject/Interface.h"
#include "NexusEventBindHelperInterface.generated.h"

class UNexusActionComponent;
// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UNexusEventBindHelperInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class NEXUSACTION_API INexusEventBindHelperInterface
{
	GENERATED_BODY()

protected:
	void BindEvents();
	void UnbindEvents(UNexusActionComponent* ActionComponent);
	virtual TArray<FNexusEventCallbackHandle> SetupEvents() = 0;

private:
	TArray<FNexusEventCallbackHandle> BoundedEventCallbackHandles;
};
