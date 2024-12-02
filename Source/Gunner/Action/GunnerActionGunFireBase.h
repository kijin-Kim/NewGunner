// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gunner/_Core/ActionSystem/GunnerAction.h"
#include "GunnerActionGunFireBase.generated.h"


/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerActionGunFireBase : public UGunnerAction
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void FireHitScan();
	
};
