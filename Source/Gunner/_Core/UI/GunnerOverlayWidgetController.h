// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerUserWidgetController.h"
#include "GunnerOverlayWidgetController.generated.h"


class UNexusProperty;
/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerOverlayWidgetController : public UGunnerUserWidgetController
{
	GENERATED_BODY()

public:
	
	virtual void InitWidgetController(APlayerState* InPlayerState) override;

	UFUNCTION()
	void OnSlotIndexValueDirty(float OldValue, float NewValue);
	UFUNCTION()
	void OnBulletValueDirty(float OldValue, float NewValue);
	UFUNCTION()
	void OnMagazineBulletValueDirty(float OldValue, float NewValue);


public:
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnSlotIndexValueChanged"))
	FGunnerUserWidgetSimpleTwoParamSignature OnSlotIndexValueDirtyDelegate;
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnBulletValueChanged"))
	FGunnerUserWidgetSimpleTwoParamSignature OnBulletValueDirtyDelegate;
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnMagazineBulletValueChanged"))
	FGunnerUserWidgetSimpleTwoParamSignature OnMagazineBulletValueDirtyDelegate;
};
