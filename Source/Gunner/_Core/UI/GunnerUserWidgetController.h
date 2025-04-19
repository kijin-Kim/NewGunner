// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GunnerUserWidgetController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGunnerUserWidgetSimpleTwoParamSignature, float, OldValue, float, NewValue);

/**
 * 
 */
UCLASS(BlueprintType)
class GUNNER_API UGunnerUserWidgetController : public UObject
{
	GENERATED_BODY()
public:
	virtual void InitWidgetController(APlayerState* InPlayerState)
	{
		PlayerState = InPlayerState;
	}

protected:
	
	UPROPERTY()
	TObjectPtr<APlayerState> PlayerState;
};
