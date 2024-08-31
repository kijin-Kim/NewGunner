// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EventManagerComponent.h"
#include "UObject/Interface.h"
#include "EventCallbackBindInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UEventCallbackBindInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GUNNER_API IEventCallbackBindInterface
{
	GENERATED_BODY()

protected:
	void BindEvents();
	void UnbindEvents();
	virtual TArray<FEventCallbackHandle> SetupEvents() = 0;
	virtual UEventManagerComponent* GetEventManagerComponent() const = 0;

private:
	TArray<FEventCallbackHandle> BoundedEventCallbackHandles;
};
