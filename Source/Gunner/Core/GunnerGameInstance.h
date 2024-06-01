// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GunnerGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UCurveTable* GetDamageCurveTable() const;
	UDataTable* GetWeaponDataTable() const;

public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UCurveTable> DamageCurveTable;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UDataTable> WeaponDataTable;
};
