// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GunnerUserWidgetController.h"
#include "GunnerOverlayWidgetController.generated.h"


struct FGunnerActionProperty;
/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerOverlayWidgetController : public UGunnerUserWidgetController
{
	GENERATED_BODY()

public:
	void OnBulletPropertyAdded(const FGunnerActionProperty& Property);
	virtual void InitWidgetController(APlayerState* PlayerState) override;

	UFUNCTION()
	void OnBulletValueChanged(float OldValue, float NewValue);

	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnBulletValueChanged"))
	FGunnerUserWidgetSimpleTwoParamSignature OnBulletValueChangedDelegate;

private:
	FGunnerActionProperty* GetPropertyFromPlayerState(APlayerState* PlayerState, FGameplayTag Tag);
};
