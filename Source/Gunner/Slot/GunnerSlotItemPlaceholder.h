// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerActionSlotActivation.h"
#include "GunnerEquippable.h"
#include "GunnerSlotItemPlaceholder.generated.h"


UCLASS()
class GUNNER_API UGunnerActionSlotActivation_MeleePlaceholder : public UGunnerActionSlotActivation
{
	GENERATED_BODY()

public:
	UGunnerActionSlotActivation_MeleePlaceholder();
	
};


UCLASS()
class GUNNER_API AGunnerEquippable_MeleePlaceholder : public AGunnerEquippable
{
	GENERATED_BODY()

public:
	AGunnerEquippable_MeleePlaceholder();
};
