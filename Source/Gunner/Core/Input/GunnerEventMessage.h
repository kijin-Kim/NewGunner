// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "GunnerEventMessage.generated.h"


/**
 * 
 */
USTRUCT(BlueprintType)
struct GUNNER_API FGunnerEventMessage
{
	GENERATED_BODY()

	FGunnerEventMessage()
		: EventTag(),
		  Instigator(nullptr)
		  , TargetActor(nullptr)
		  , InputActionValue()
		  , EventDataObject(nullptr)
	{
	}

	FGunnerEventMessage(FGameplayTag InEventTag, AActor* InInstigator, AActor* InTargetActor, const FInputActionValue& InInputActionValue, UObject* InEventDataObject)
		: EventTag(InEventTag),
		  Instigator(InInstigator)
		  , TargetActor(InTargetActor)
		  , InputActionValue(InInputActionValue)
		  , EventDataObject(InEventDataObject)
	{
	}

public:
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag EventTag;
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> Instigator;
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> TargetActor;
	UPROPERTY(BlueprintReadOnly)
	FInputActionValue InputActionValue;
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UObject> EventDataObject;
};

USTRUCT()
struct FGunnerReplicatedInputActionValue
{
	GENERATED_BODY()

	FGunnerReplicatedInputActionValue() = default;

	FGunnerReplicatedInputActionValue(const FInputActionValue& InputActionValue) :
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
struct GUNNER_API FGunnerEventMessageReplicated
{
	GENERATED_BODY()
	FGunnerEventMessageReplicated()
		: EventTag(),
		  Instigator(nullptr)
		  , TargetActor(nullptr)
		  , ReplicatedInputActionValue()
		  , EventDataObject(nullptr)
	{
	}

	FGunnerEventMessageReplicated(FGameplayTag InEventTag, AActor* InInstigator, AActor* InTargetActor, const FInputActionValue& InInputActionValue, UObject* InEventDataObject)
		: EventTag(InEventTag),
		  Instigator(InInstigator)
		  , TargetActor(InTargetActor)
		  , ReplicatedInputActionValue(InInputActionValue)
		  , EventDataObject(InEventDataObject)
	{
	}

	FGunnerEventMessageReplicated(const FGunnerEventMessage& EventMessage)
		: EventTag(EventMessage.EventTag),
		  Instigator(EventMessage.Instigator)
		  , TargetActor(EventMessage.TargetActor)
		  , ReplicatedInputActionValue(EventMessage.InputActionValue)
		  , EventDataObject(EventMessage.EventDataObject)
	{
	}

	FGunnerEventMessage ToEventMessage() const
	{
		return FGunnerEventMessage(EventTag, Instigator.Get(), TargetActor.Get(), FInputActionValue(ReplicatedInputActionValue.ValueType, ReplicatedInputActionValue.Value), EventDataObject.Get());
	}

private:
	UPROPERTY()
	FGameplayTag EventTag;
	UPROPERTY()
	TObjectPtr<AActor> Instigator;
	UPROPERTY()
	TObjectPtr<AActor> TargetActor;
	UPROPERTY()
	FGunnerReplicatedInputActionValue ReplicatedInputActionValue;
	UPROPERTY()
	TObjectPtr<UObject> EventDataObject;
};
