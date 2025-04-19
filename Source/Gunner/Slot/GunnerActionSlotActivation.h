// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerSlotItem.h"
#include "Gunner/Action/GunnerActionSlotItemBase.h"
#include "SideEffect/NexusSideEffect.h"
#include "GunnerActionSlotActivation.generated.h"

UCLASS()
class GUNNER_API UGunnerSlotIndexChangeSideEffect : public UNexusSideEffect
{
	GENERATED_BODY()

public:
	UGunnerSlotIndexChangeSideEffect();
};


UCLASS()
class GUNNER_API UGunnerActionSlotActivation : public UGunnerActionSlotItemBase
{
	GENERATED_BODY()

protected:
	UGunnerActionSlotActivation();
	virtual bool OnCanTriggerAction() const override;
	virtual void OnTriggerAction() override;

private:
	EGunnerSlotType GetCurrentSlotType() const;
};


