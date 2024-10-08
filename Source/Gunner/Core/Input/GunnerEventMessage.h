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
		: Instigator(nullptr)
		  , TargetActor(nullptr)
		  , InputActionValue()
		  , EventDataObject(nullptr)
	{
	}

	FGunnerEventMessage(AActor* InInstigator, AActor* InTargetActor, const FInputActionValue& InInputActionValue, UObject* InEventDataObject)
		: Instigator(InInstigator)
		  , TargetActor(InTargetActor)
		  , InputActionValue(InInputActionValue)
		  , EventDataObject(InEventDataObject)
	{
	}

public:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> Instigator;
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> TargetActor;
	UPROPERTY(BlueprintReadWrite)
	FInputActionValue InputActionValue;
	UPROPERTY(BlueprintReadWrite)
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
		: Instigator(nullptr)
		  , TargetActor(nullptr)
		  , ReplicatedInputActionValue()
		  , EventDataObject(nullptr)
	{
	}

	FGunnerEventMessageReplicated(AActor* InInstigator, AActor* InTargetActor, const FInputActionValue& InInputActionValue, UObject* InEventDataObject)
		: Instigator(InInstigator)
		  , TargetActor(InTargetActor)
		  , ReplicatedInputActionValue(InInputActionValue)
		  , EventDataObject(InEventDataObject)
	{
	}

	FGunnerEventMessageReplicated(const FGunnerEventMessage& EventMessage)
		: Instigator(EventMessage.Instigator)
		  , TargetActor(EventMessage.TargetActor)
		  , ReplicatedInputActionValue(EventMessage.InputActionValue)
		  , EventDataObject(EventMessage.EventDataObject)
	{
	}

	FGunnerEventMessage ToEventMessage() const
	{
		return FGunnerEventMessage(Instigator.Get(), TargetActor.Get(), FInputActionValue(ReplicatedInputActionValue.ValueType, ReplicatedInputActionValue.Value), EventDataObject.Get());
	}

private:
	UPROPERTY()
	TObjectPtr<AActor> Instigator;
	UPROPERTY()
	TObjectPtr<AActor> TargetActor;
	UPROPERTY()
	FGunnerReplicatedInputActionValue ReplicatedInputActionValue;
	UPROPERTY()
	TObjectPtr<UObject> EventDataObject;
};
