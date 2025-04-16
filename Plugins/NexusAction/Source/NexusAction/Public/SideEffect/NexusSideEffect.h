// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NexusProperty.h"
#include "NexusSideEffectInstanceHandle.h"
#include "Action/NexusAgentInfo.h"
#include "Prediction/NexusPrediction.h"
#include "UObject/Object.h"
#include "NexusSideEffect.generated.h"


struct FNexusAgentInfo;

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

USTRUCT(BlueprintType)
struct FNexusGameplayTagMod
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Operation")
	FGameplayTagContainer TagsToGrant;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Operation")
	FGameplayTagContainer TagsToRevoke;
};

/**
 * 
 */
UCLASS()
class NEXUSACTION_API UNexusSideEffect : public UDataAsset
{
	GENERATED_BODY()


public:
	UPROPERTY(EditAnywhere)
	ESideEffectDurationType DurationType = ESideEffectDurationType::Instant;
	UPROPERTY(EditAnywhere, meta = (EditCondition = "DurationType == ESideEffectDurationType::Duration", EditConditionHides))
	float Duration = 0.0f;
	UPROPERTY(EditAnywhere, meta = (EditCondition = "DurationType != ESideEffectDurationType::Instant", EditConditionHides))
	float Interval = 0.0f;


	UPROPERTY(EditAnywhere, Category = "Property")
	TArray<FNexusPropertyMod> Modifiers;

	UPROPERTY(EditAnywhere, Category = "GameplayTag")
	TArray<FNexusGameplayTagMod> TagModifiers;
};


UCLASS()
class UNexusSideEffectInfinite : public UNexusSideEffect
{
	GENERATED_BODY()

public:
	UNexusSideEffectInfinite()
	{
		DurationType = ESideEffectDurationType::Infinite;
	}
};
