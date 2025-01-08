// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AsyncAction/GunnerActionNetPrediction.h"
#include "Gunner/_Core/ActionSystem/GunnerActionProperty.h"
#include "UObject/Object.h"
#include "GunnerActionSideEffect.generated.h"


UENUM()
enum class ESideEffectDurationType
{
	Instant,
	Duration,
	Infinite
};

USTRUCT()
struct FGunnerPropertyModifier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Property Operation")
	FGameplayTag PropertyTag;
	UPROPERTY(EditAnywhere, Category = "Property Operation")
	EGunnerActionPropertyCalculationType CalculationType = EGunnerActionPropertyCalculationType::None;
	UPROPERTY(EditAnywhere, Category = "Property Operation", meta = (EditCondition = "CalculationType != EGunnerActionPropertyCalculationType::None", EditConditionHides))
	EGunnerActionPropertyOperator Operator = EGunnerActionPropertyOperator::Add;

	UPROPERTY(EditAnywhere, Category = "Property Operation", meta = (EditCondition = "(CalculationType != EGunnerActionPropertyCalculationType::None) && (CalculationType == EGunnerActionPropertyCalculationType::Direct)", EditConditionHides))
	float DirectValue;
	UPROPERTY(EditAnywhere, Category = "Property Operation", meta = (EditCondition = "(CalculationType != EGunnerActionPropertyCalculationType::None) && (CalculationType == EGunnerActionPropertyCalculationType::PropertyBased)", EditConditionHides))
	FGameplayTag BaseProperty;
	UPROPERTY(EditAnywhere, Category = "Property Operation", meta = (EditCondition = "(CalculationType != EGunnerActionPropertyCalculationType::None) && (CalculationType == EGunnerActionPropertyCalculationType::FromOutside)", EditConditionHides))
	FGameplayTag InjectedValueTag;
};

/**
 * 
 */
UCLASS(Blueprintable)
class GUNNER_API UGunnerActionSideEffect : public UObject
{
	GENERATED_BODY()

public:
	void OnApplied(FGunnerActionNetPredictionHandle PredictionHandle, bool bHasAuthority);
	void OnTick(float DeltaTime, bool bHasAuthority);
	void OnRemoved();

	UFUNCTION(BlueprintCallable)
	void SetInjectedValue(FGameplayTag Tag, float Value);
	

private:
	void ApplyModifier(const FGunnerPropertyModifier& Modifier, FGunnerActionNetPredictionHandle PredictionHandle, bool bHasAuthority);
	void ApplyAllModifiers(FGunnerActionNetPredictionHandle PredictionHandle, bool bHasAuthority);


public:
	UPROPERTY(EditAnywhere)
	ESideEffectDurationType DurationType = ESideEffectDurationType::Instant;
	UPROPERTY(EditAnywhere, meta = (EditCondition = "DurationType == ESideEffectDurationType::Duration", EditConditionHides))
	float Duration;
	UPROPERTY(EditAnywhere, meta = (EditCondition = "DurationType != ESideEffectDurationType::Instant", EditConditionHides))
	float Interval;


	UPROPERTY(EditAnywhere)
	TArray<FGunnerPropertyModifier> Modifiers;

	float RemainingDuration = 0.0f;
	float ElapsedTime = 0.0f;

	TMap<FGameplayTag, float> InjectedValues;

	TArray<FGunnerActionPropertyOperationHandle> OperationHandles;
};
