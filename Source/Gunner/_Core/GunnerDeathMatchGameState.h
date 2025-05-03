// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerGameState.h"
#include "GunnerDeathMatchGameState.generated.h"

/**
 * 
 */
UCLASS()
class GUNNER_API AGunnerDeathMatchGameState : public AGunnerGameState
{
	GENERATED_BODY()
public:
	virtual TArray<int32> DetermineWinners() const override;

};
