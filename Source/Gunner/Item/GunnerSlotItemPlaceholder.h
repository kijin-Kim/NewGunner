// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerAction_EquipBase.h"
#include "GunnerEquipmentItem.h"
#include "GunnerSlotItemPlaceholder.generated.h"


UCLASS()
class GUNNER_API UGunnerActionSlotActivation_MeleePlaceholder : public UGunnerAction_EquipBase
{
	GENERATED_BODY()

public:
	UGunnerActionSlotActivation_MeleePlaceholder();
	virtual void OnTriggerAction() override;
	
};


UCLASS()
class GUNNER_API AGunnerEquippable_MeleePlaceholder : public AGunnerEquipmentItem
{
	GENERATED_BODY()

public:
	AGunnerEquippable_MeleePlaceholder();
};
