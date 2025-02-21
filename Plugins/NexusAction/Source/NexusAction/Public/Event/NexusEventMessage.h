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
		  EventDataObject(nullptr)
	{
	}

	FNexusEventMessage(FGameplayTag InEventTag, AController* InInstigator, AActor* InTargetActor, const FInputActionValue& InInputActionValue, UObject* InEventDataObject)
		: EventTag(InEventTag),
		  Instigator(InInstigator),
		  TargetActor(InTargetActor),
		  InputActionValue(InInputActionValue),
		  EventDataObject(InEventDataObject)
	{
	}

public:
	UPROPERTY(BlueprintReadWrite)
	FGameplayTag EventTag;
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AController> Instigator;
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> TargetActor;
	UPROPERTY(BlueprintReadWrite)
	FInputActionValue InputActionValue;
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UObject> EventDataObject;
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
		  EventDataObject(nullptr)
	{
	}

	FNexusEventMessageReplicated(FGameplayTag InEventTag, AController* InInstigator, AActor* InTargetActor, const FInputActionValue& InInputActionValue, UObject* InEventDataObject)
		: EventTag(InEventTag),
		  Instigator(InInstigator),
		  TargetActor(InTargetActor),
		  ReplicatedInputActionValue(InInputActionValue),
		  EventDataObject(InEventDataObject)
	{
	}

	FNexusEventMessageReplicated(const FNexusEventMessage& EventMessage)
		: EventTag(EventMessage.EventTag),
		  Instigator(EventMessage.Instigator),
		  TargetActor(EventMessage.TargetActor),
		  ReplicatedInputActionValue(EventMessage.InputActionValue),
		  EventDataObject(EventMessage.EventDataObject)
	{
	}

	FNexusEventMessage ToEventMessage() const
	{
		return FNexusEventMessage(EventTag, Instigator.Get(), TargetActor.Get(), FInputActionValue(ReplicatedInputActionValue.ValueType, ReplicatedInputActionValue.Value), EventDataObject.Get());
	}

private:
	UPROPERTY()
	FGameplayTag EventTag;
	UPROPERTY()
	TObjectPtr<AController> Instigator;
	UPROPERTY()
	TObjectPtr<AActor> TargetActor;
	UPROPERTY()
	FNexusRepInputActionValue ReplicatedInputActionValue;
	UPROPERTY()
	TObjectPtr<UObject> EventDataObject;
};
