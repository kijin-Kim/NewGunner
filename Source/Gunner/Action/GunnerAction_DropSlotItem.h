// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Action/NexusAction.h"
#include "Gunner/Item/GunnerInventoryManagerComponent.h"
#include "Gunner/Item/GunnerSlotItem.h"
#include "GunnerAction_DropSlotItem.generated.h"


class AGunnerSlotItemPickup;
class UGunnerInventoryManagerComponent;
/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerAction_DropSlotItem : public UNexusAction
{
	GENERATED_BODY()

public:
	UGunnerAction_DropSlotItem();
	virtual void OnAddAction() override;
	virtual bool OnCanTriggerAction() const override;
	virtual void OnTriggerAction() override;

private:
	EGunnerSlotType GetCurrentSlotType() const;


protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<AGunnerSlotItemPickup> PickupClass;

private:
	UPROPERTY()
	TObjectPtr<UGunnerInventoryManagerComponent> InventoryManagerComponent;
	UPROPERTY()
	TObjectPtr<UNexusActionComponent> ActionComponent;
};

UCLASS()
class GUNNER_API UGunnerActionCycleSlotItem : public UNexusAction
{
	GENERATED_BODY()
	
public:
	UGunnerActionCycleSlotItem();
	virtual void OnAddAction() override;
	virtual bool OnCanTriggerAction() const override;
	virtual void OnTriggerAction() override;

private:
	UPROPERTY()
	TObjectPtr<UGunnerInventoryManagerComponent> InventoryManagerComponent;
};