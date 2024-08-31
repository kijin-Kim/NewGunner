// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GunnerStateMachineTransitionData.generated.h"


class UGunnerStateComponent;
class UGunnerStateMachineTransitionCondition;

USTRUCT()
struct FGunnerStateMachineToStateAndConditionClass
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGunnerStateMachineTransitionCondition> TransitionConditionClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGunnerStateComponent> ToStateComponentClass;
};


USTRUCT()
struct FGunnerStateMachineTransitionDataEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGunnerStateComponent> FromStateComponentClass;
	UPROPERTY(EditAnywhere, meta = (TitleProperty = "ToStateComponentClass"))
	TArray<FGunnerStateMachineToStateAndConditionClass> ToStateAndConditionClasses;
};


/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerStateMachineTransitionData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, meta = (TitleProperty = "FromStateComponentClass"))
	TArray<FGunnerStateMachineTransitionDataEntry> TransitionData;
};
