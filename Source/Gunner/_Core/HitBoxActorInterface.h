// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HitBoxActorInterface.generated.h"


struct FHitBox;
// This class does not need to be modified.
UINTERFACE()
class UHitBoxActorInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GUNNER_API IHitBoxActorInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual TArray<FHitBox> CollectAndGetHitBoxes() = 0;
	
};
