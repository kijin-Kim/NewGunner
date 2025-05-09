// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTags.h"
#include "InputActionValue.h"
#include "NexusEventMessage.generated.h"


/**
 * 
 */
USTRUCT(BlueprintType)
struct NEXUSACTION_API FNexusEventMessage
{
	GENERATED_BODY()

	FNexusEventMessage()
		: EventTag(),
		  Instigator(nullptr),
		  TargetActor(nullptr),
		  InputActionValue(),
		  Amount(0.0f),
		  Location(FVector::ZeroVector),
		  Normal(FVector::ForwardVector)
	{
	}

	FNexusEventMessage(FGameplayTag InEventTag, AActor* InInstigator, AActor* InTargetActor, const FInputActionValue& InInputActionValue, float InAmount, FVector InLocation, FVector InNormal, const TArray<FHitResult>& InHitResults)
		: EventTag(InEventTag),
		  Instigator(InInstigator),
		  TargetActor(InTargetActor),
		  InputActionValue(InInputActionValue),
		  Amount(InAmount),
		  Location(InLocation),
		  Normal(InNormal),
		  HitResults(InHitResults)
	{
	}

public:
	UPROPERTY(BlueprintReadWrite)
	FGameplayTag EventTag;
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> Instigator;
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> TargetActor;
	UPROPERTY(BlueprintReadWrite)
	FInputActionValue InputActionValue;

	UPROPERTY(BlueprintReadWrite)
	float Amount;

	UPROPERTY(BlueprintReadWrite)
	FVector_NetQuantize Location;

	UPROPERTY(BlueprintReadWrite)
	FVector_NetQuantizeNormal Normal;

	UPROPERTY(BlueprintReadWrite)
	TArray<FHitResult> HitResults;
};

USTRUCT()
struct FNexusRepInputActionValue
{
	GENERATED_BODY()

	FNexusRepInputActionValue() = default;

	FNexusRepInputActionValue(const FInputActionValue& InputActionValue) :
		Value({InputActionValue[0], InputActionValue[1], InputActionValue[2]}),
		ValueType(InputActionValue.GetValueType())
	{
	}


	UPROPERTY()
	FVector Value = FVector::ZeroVector;
	UPROPERTY()
	EInputActionValueType ValueType = EInputActionValueType::Boolean;
};

USTRUCT(BlueprintType)
struct NEXUSACTION_API FNexusEventMessageReplicated
{
	GENERATED_BODY()
	FNexusEventMessageReplicated()
		: EventTag(),
		  Instigator(nullptr),
		  TargetActor(nullptr),
		  ReplicatedInputActionValue(),
		  Amount(0.0f)


	{
	}

	FNexusEventMessageReplicated(FGameplayTag InEventTag, AActor* InInstigator, AActor* InTargetActor, const FInputActionValue& InInputActionValue, float InAmount, FVector InLocation, FVector InNormal, const TArray<FHitResult>& InHitResults)
		: EventTag(InEventTag),
		  Instigator(InInstigator),
		  TargetActor(InTargetActor),
		  ReplicatedInputActionValue(InInputActionValue),
		  Amount(InAmount),
		  Location(InLocation),
		  Normal(InNormal),
		  HitResults(InHitResults)
	{
	}

	explicit FNexusEventMessageReplicated(const FNexusEventMessage& EventMessage)
		: EventTag(EventMessage.EventTag),
		  Instigator(EventMessage.Instigator),
		  TargetActor(EventMessage.TargetActor),
		  ReplicatedInputActionValue(EventMessage.InputActionValue),
		  Amount(0.0f),
		  Location(EventMessage.Location),
		  Normal(EventMessage.Normal),
		  HitResults(EventMessage.HitResults)
	{
	}

	FNexusEventMessage ToEventMessage() const
	{
		return FNexusEventMessage(EventTag, Instigator, TargetActor, ReplicatedInputActionValue.Value, Amount, Location, Normal, HitResults);
	}

private:
	UPROPERTY()
	FGameplayTag EventTag;
	UPROPERTY()
	TObjectPtr<AActor> Instigator;
	UPROPERTY()
	TObjectPtr<AActor> TargetActor;
	UPROPERTY()
	FNexusRepInputActionValue ReplicatedInputActionValue;

	UPROPERTY()
	float Amount;

	UPROPERTY()
	FVector_NetQuantize Location = FVector::ZeroVector;

	UPROPERTY()
	FVector_NetQuantizeNormal Normal = FVector::ForwardVector;

	UPROPERTY()
	TArray<FHitResult> HitResults;
};
