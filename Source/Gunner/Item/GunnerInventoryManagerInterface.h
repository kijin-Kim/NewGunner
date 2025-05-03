// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GunnerInventoryManagerInterface.generated.h"

class UGunnerInventoryManagerComponent;
// This class does not need to be modified.
UINTERFACE()
class GUNNER_API UGunnerInventoryManagerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GUNNER_API IGunnerInventoryManagerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual UGunnerInventoryManagerComponent* GetInventoryManagerComponent() const = 0;
};
