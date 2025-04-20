// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerEquippable.h"
#include "GunnerGun.generated.h"

class UGunnerLocomotionAnimSet;
class UNexusAnimMontagePlayerComponent;

UCLASS()
class GUNNER_API AGunnerGun : public AGunnerEquippable
{
	GENERATED_BODY()

public:
	AGunnerGun();

	//~ Begin AActor
	virtual void OnConstruction(const FTransform& Transform) override;
	//~ End AActor

	//~ Begin AGunnerSlotItem
	virtual void OnActivated() override;
	virtual void OnDeactivated() override;
	//~ End AGunnerSlotItem


protected:
	UPROPERTY(EditAnywhere)
	float Magazine;
	UPROPERTY(EditAnywhere)
	float Reserve;

	float BulletCount;
	float MagazineBulletCount;
	float MaxBulletPerMagazineCount;
};
