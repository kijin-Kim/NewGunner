// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GunnerUserWidgetController.h"
#include "GunnerOverlayWidgetController.generated.h"


class UGunnerActionProperty;
/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerOverlayWidgetController : public UGunnerUserWidgetController
{
	GENERATED_BODY()

public:
	virtual void InitWidgetController(APlayerState* PlayerState) override;

	UFUNCTION()
	void OnBulletValueChanged(float OldValue, float NewValue);
	UFUNCTION()
	void OnMagazineBulletValueChanged(float OldValue, float NewValue);
	

public:
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnBulletValueChanged"))
	FGunnerUserWidgetSimpleTwoParamSignature OnBulletValueChangedDelegate;
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnMagazineBulletValueChanged"))
	FGunnerUserWidgetSimpleTwoParamSignature OnMagazineBulletValueChangedDelegate;
};
