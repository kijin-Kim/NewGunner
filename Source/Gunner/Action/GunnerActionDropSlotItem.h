// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Action/NexusAction.h"
#include "Gunner/Slot/GunnerSlotManagerComponent.h"
#include "GunnerActionDropSlotItem.generated.h"


class AGunnerSlotItemPickup;
class UGunnerSlotManagerComponent;
/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerActionDropSlotItem : public UNexusAction
{
	GENERATED_BODY()

public:
	UGunnerActionDropSlotItem();
	virtual void OnActionAdded() override;
	virtual bool OnCanTriggerAction() const override;
	virtual void OnTriggerAction() override;


protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<AGunnerSlotItemPickup> PickupClass;

private:
	UPROPERTY()
	TObjectPtr<UGunnerSlotManagerComponent> SlotManagerComponent;
};

UCLASS()
class GUNNER_API UGunnerActionCycleSlotItem : public UNexusAction
{
	GENERATED_BODY()
	
public:
	UGunnerActionCycleSlotItem();
	virtual void OnActionAdded() override;
	virtual bool OnCanTriggerAction() const override;
	virtual void OnTriggerAction() override;

private:
	UPROPERTY()
	TObjectPtr<UGunnerSlotManagerComponent> SlotManagerComponent;
};