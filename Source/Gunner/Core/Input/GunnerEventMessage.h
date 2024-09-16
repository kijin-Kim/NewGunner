// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GunnerEventMessage.generated.h"

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
		  , ReplicatedInputActionValue()
		  , EventDataObject(nullptr)
	{
	}

	FGunnerEventMessage(AActor* InInstigator, AActor* InTargetActor, const FInputActionValue& InInputActionValue, UObject* InEventDataObject)
		: Instigator(InInstigator)
		  , TargetActor(InTargetActor)
		  , ReplicatedInputActionValue(InInputActionValue)
		  , EventDataObject(InEventDataObject)
	{
	}

	AActor* GetInstigator() const { return Instigator.Get(); }
	AActor* GetTargetActor() const { return TargetActor.Get(); }
	FInputActionValue GetInputActionValue() const { return FInputActionValue(ReplicatedInputActionValue.ValueType, ReplicatedInputActionValue.Value); }
	UObject* GetEventDataObject() const { return EventDataObject.Get(); }

private:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AActor> Instigator;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AActor> TargetActor;
	UPROPERTY()
	FGunnerReplicatedInputActionValue ReplicatedInputActionValue;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UObject> EventDataObject;
};
