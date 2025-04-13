// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Prediction/NexusPrediction.h"
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

	FNexusPropertyMod()
		: PropertyTag(FGameplayTag::EmptyTag),
		  CalculationType(ENexusPropertyCalculationType::None),
		  Operator(ENexusPropertyOperator::Add),
		  DirectValue(0.0f),
		  BaseProperty(FGameplayTag::EmptyTag),
		  InjectedValueTag(FGameplayTag::EmptyTag)
	{
	}

	UPROPERTY(EditAnywhere, Category = "Property Operation")
	FGameplayTag PropertyTag;
	UPROPERTY(EditAnywhere, Category = "Property Operation")
	ENexusPropertyCalculationType CalculationType;
	UPROPERTY(EditAnywhere, Category = "Property Operation", meta = (EditCondition = "CalculationType != ENexusPropertyCalculationType::None", EditConditionHides))
	ENexusPropertyOperator Operator;

	UPROPERTY(EditAnywhere, Category = "Property Operation", meta = (EditCondition = "(CalculationType != ENexusPropertyCalculationType::None) && (CalculationType == ENexusPropertyCalculationType::Direct)", EditConditionHides))
	float DirectValue;
	UPROPERTY(EditAnywhere, Category = "Property Operation", meta = (EditCondition = "(CalculationType != ENexusPropertyCalculationType::None) && (CalculationType == ENexusPropertyCalculationType::PropertyBased)", EditConditionHides))
	FGameplayTag BaseProperty;
	UPROPERTY(EditAnywhere, Category = "Property Operation", meta = (EditCondition = "(CalculationType != ENexusPropertyCalculationType::None) && (CalculationType == ENexusPropertyCalculationType::FromOutside)", EditConditionHides))
	FGameplayTag InjectedValueTag;
};

USTRUCT()
struct FNexusGameplayTagMod
{
	GENERATED_BODY()
	

	UPROPERTY(EditAnywhere, Category = "Tag Operation")
	FGameplayTagContainer TagsToGrant;
	UPROPERTY(EditAnywhere, Category = "Tag Operation")
	FGameplayTagContainer TagsToRevoke;
};

/**
 * 
 */
UCLASS(Blueprintable)
class NEXUSACTION_API UNexusSideEffect : public UObject
{
	GENERATED_BODY()

public:
#ifdef WITH_EDITOR
	
#endif
	void OnApplied(FNexusPredictionTag PredictionTag, bool bHasAuthority);
	void OnTick(float DeltaTime, bool bHasAuthority);
	void OnRemoved();

	UFUNCTION(BlueprintCallable)
	void SetInjectedValue(FGameplayTag Tag, float Value);

	const TMap<FGameplayTag, float>& GetInjectedValues() const { return InjectedValues; }
	float GetRemainingDuration() const { return RemainingDuration; }
	float GetElapsedTime() const { return ElapsedTime; }
	int32 GetAppliedCount() const { return AppliedCount; }

private:
	void ApplyPropertyModifier(const FNexusPropertyMod& Modifier, FNexusPredictionTag PredictionTag, bool bHasAuthority);
	void ApplyTagModifier(const FNexusGameplayTagMod& Modifier, FNexusPredictionTag PredictionTag, bool bHasAuthority);
	void ApplyAllModifiers(FNexusPredictionTag PredictionTag, bool bHasAuthority);

public:
	UPROPERTY(EditAnywhere)
	ESideEffectDurationType DurationType = ESideEffectDurationType::Instant;
	UPROPERTY(EditAnywhere, meta = (EditCondition = "DurationType == ESideEffectDurationType::Duration", EditConditionHides))
	float Duration;
	UPROPERTY(EditAnywhere, meta = (EditCondition = "DurationType != ESideEffectDurationType::Instant", EditConditionHides))
	float Interval;


	UPROPERTY(EditAnywhere, Category = "Property")
	TArray<FNexusPropertyMod> Modifiers;

	UPROPERTY(EditAnywhere, Category = "GameplayTag")
	TArray<FNexusGameplayTagMod> TagModifiers;


private:
	TMap<FGameplayTag, float> InjectedValues;
	TArray<FNexusPropertyOperationHandle> OperationHandles;
	
	float RemainingDuration = 0.0f;
	float ElapsedTime = 0.0f;
	int32 AppliedCount;
};
