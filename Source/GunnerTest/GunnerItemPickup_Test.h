// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gunner/Action/GunnerAction_DropSlotItem.h"
#include "Gunner/_Core/GunnerSlotItemPickup.h"
#include "GunnerItemPickup_Test.generated.h"


UCLASS()
class AGunnerItemPickup_Test : public AGunnerSlotItemPickup
{
	GENERATED_BODY()

public:
	AGunnerItemPickup_Test();
	virtual bool CanPickup_Implementation(AActor* OtherActor, UActorComponent* OtherComponent) const override;
	void SetPickupEnabled(bool bEnabled);
};


UCLASS()
class UGunnerActionTestDropSlotItem : public UGunnerAction_DropSlotItem
{
	GENERATED_BODY()

public:
	UGunnerActionTestDropSlotItem();
};
