// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gunner/Action/GunnerActionDropSlotItem.h"
#include "Gunner/_Core/GunnerSlotItemPickup.h"
#include "GunnerTestSlotItemPickup.generated.h"


UCLASS()
class AGunnerTestSlotItemPickup : public AGunnerSlotItemPickup
{
	GENERATED_BODY()

public:
	AGunnerTestSlotItemPickup();
	virtual bool CanPickup_Implementation(AActor* OtherActor, UActorComponent* OtherComponent) const override;
	void SetPickupEnabled(bool bEnabled);
};


UCLASS()
class UGunnerActionTestDropSlotItem : public UGunnerActionDropSlotItem
{
	GENERATED_BODY()

public:
	UGunnerActionTestDropSlotItem();
};
