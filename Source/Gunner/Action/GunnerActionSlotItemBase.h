// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Action/NexusAction.h"
#include "GunnerActionSlotItemBase.generated.h"

class AGunnerSlotItem;
/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerActionSlotItemBase : public UNexusAction
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	AGunnerSlotItem* GetSlotItem() const;
};
