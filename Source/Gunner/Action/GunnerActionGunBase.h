// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Action/NexusAction.h"
#include "GunnerActionGunBase.generated.h"

class AGunnerGun;
/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerActionGunBase : public UNexusAction
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	AGunnerGun* GetGun() const;
};
