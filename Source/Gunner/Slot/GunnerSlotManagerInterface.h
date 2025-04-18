// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GunnerSlotManagerInterface.generated.h"

class UGunnerSlotManagerComponent;
// This class does not need to be modified.
UINTERFACE()
class GUNNER_API UGunnerSlotManagerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GUNNER_API IGunnerSlotManagerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual UGunnerSlotManagerComponent* GetSlotManagerComponent() const = 0;
};
