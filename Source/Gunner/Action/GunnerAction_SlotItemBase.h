// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Action/NexusAction.h"
#include "GunnerAction_SlotItemBase.generated.h"

class AGunnerSlotItem;
/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerAction_SlotItemBase : public UNexusAction
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	AActor* GetSourceObjectActor() const;
	
};
