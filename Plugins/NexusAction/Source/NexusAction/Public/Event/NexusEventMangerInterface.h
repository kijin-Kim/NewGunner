// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NexusEventMangerInterface.generated.h"

class UNexusEventManagerComponent;
// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UNexusEventManagerInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

/**
 * 
 */
class NEXUSACTION_API INexusEventManagerInterface
{
	GENERATED_IINTERFACE_BODY()
	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual UNexusEventManagerComponent* GetEventManagerComponent() const = 0;
};
