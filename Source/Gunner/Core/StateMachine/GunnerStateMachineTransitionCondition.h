// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GunnerStateMachineTransitionCondition.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class GUNNER_API UGunnerStateMachineTransitionCondition : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent)
	bool ShouldTransit() const;
};
