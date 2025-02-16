// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NexusPrediction.h"
#include "NexusProperty.h"
#include "UObject/Object.h"
#include "NexusSideEffect.generated.h"


UENUM()
enum class ESideEffectDurationType
{
	Instant,
	Duration,
	Infinite
};

USTRUCT()
struct FNexusPropertyMod
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Property Operation")
	FGameplayTag PropertyTag;
	UPROPERTY(EditAnywhere, Category = "Property Operation")
	ENexusPropertyCalculationType CalculationType = ENexusPropertyCalculationType::None;
	UPROPERTY(EditAnywhere, Category = "Property Operation", meta = (EditCondition = "CalculationType != ENexusPropertyCalculationType::None", EditConditionHides))
	ENexusPropertyOperator Operator = ENexusPropertyOperator::Add;

	UPROPERTY(EditAnywhere, Category = "Property Operation", meta = (EditCondition = "(CalculationType != ENexusPropertyCalculationType::None) && (CalculationType == ENexusPropertyCalculationType::Direct)", EditConditionHides))
	float DirectValue;
	UPROPERTY(EditAnywhere, Category = "Property Operation", meta = (EditCondition = "(CalculationType != ENexusPropertyCalculationType::None) && (CalculationType == ENexusPropertyCalculationType::PropertyBased)", EditConditionHides))
	FGameplayTag BaseProperty;
	UPROPERTY(EditAnywhere, Category = "Property Operation", meta = (EditCondition = "(CalculationType != ENexusPropertyCalculationType::None) && (CalculationType == ENexusPropertyCalculationType::FromOutside)", EditConditionHides))
	FGameplayTag InjectedValueTag;
};

/**
 * 
 */
UCLASS(Blueprintable)
class NEXUSACTION_API UNexusSideEffect : public UObject
{
	GENERATED_BODY()

public:
	void OnApplied(FNexusPredictionTag PredictionHandle, bool bHasAuthority);
	void OnTick(float DeltaTime, bool bHasAuthority);
	void OnRemoved();

	UFUNCTION(BlueprintCallable)
	void SetInjectedValue(FGameplayTag Tag, float Value);
	

private:
	void ApplyModifier(const FNexusPropertyMod& Modifier, FNexusPredictionTag PredictionHandle, bool bHasAuthority);
	void ApplyAllModifiers(FNexusPredictionTag PredictionHandle, bool bHasAuthority);


public:
	UPROPERTY(EditAnywhere)
	ESideEffectDurationType DurationType = ESideEffectDurationType::Instant;
	UPROPERTY(EditAnywhere, meta = (EditCondition = "DurationType == ESideEffectDurationType::Duration", EditConditionHides))
	float Duration;
	UPROPERTY(EditAnywhere, meta = (EditCondition = "DurationType != ESideEffectDurationType::Instant", EditConditionHides))
	float Interval;


	UPROPERTY(EditAnywhere)
	TArray<FNexusPropertyMod> Modifiers;

	float RemainingDuration = 0.0f;
	float ElapsedTime = 0.0f;

	TMap<FGameplayTag, float> InjectedValues;

	TArray<FNexusPropertyOperationHandle> OperationHandles;
};
