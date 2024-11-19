// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Gunner/_Core/ActionSystem/GunnerActionSideEffectDefinitionHandle.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "UObject/Object.h"
#include "GunnerActionProperty.generated.h"


DECLARE_DELEGATE_TwoParams(FOnGunnerActionPropertyValueChangedSignature, float OldValue, float NewValue);
DECLARE_DELEGATE_OneParam(FOnGunnerActionPropertyCountChangedSignature, const struct FGunnerActionProperty& NewProperty);


UENUM()
enum class EGunnerActionPropertyOperator
{
	Add,
	Subtract,
	Multiply,
	Divide,
	Override
};


USTRUCT()
struct FGunnerActionPropertyOperation
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EGunnerActionPropertyOperator Operator;
	UPROPERTY(EditAnywhere)
	float Operand;
	UPROPERTY(EditAnywhere)
	float Duration;

	FGunnerActionSideEffectDefinitionHandle SideEffectDefinitionHandle;
};

/**
 * 
 */

struct FGunnerActionPropertyArray;

USTRUCT()
struct FGunnerActionProperty : public FFastArraySerializerItem
{
	GENERATED_USTRUCT_BODY()

public:
	bool operator==(const FGunnerActionProperty& Other) const
	{
		return Tag == Other.Tag;
	}

	bool operator!=(const FGunnerActionProperty& Other) const
	{
		return !(*this == Other);
	}

	void MarkPropertyDirty();
	void Evaluate(const TArray<FGunnerActionPropertyOperation>& PropertyOperation, float& TargetValue);
	void PostReplicatedAdd(const FGunnerActionPropertyArray& InArraySerializer);
	void PreReplicatedRemove(const FGunnerActionPropertyArray& InArraySerializer);
	void PostReplicatedChange(const FGunnerActionPropertyArray& InArraySerializer);

public:
	UPROPERTY()
	FGameplayTag Tag;
	UPROPERTY()
	float StaticValue;
	UPROPERTY()
	float DynamicValue;
	TArray<FGunnerActionPropertyOperation> StaticOperations;
	TArray<FGunnerActionPropertyOperation> DynamicOperations;

	FOnGunnerActionPropertyValueChangedSignature OnGunnerActionPropertyValueChangedDelegate;
};

USTRUCT()
struct FGunnerActionPropertyArray : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TArray<FGunnerActionProperty> Items;

	void AuthAdd(const FGunnerActionProperty& Property);
	void AuthRemove(FGameplayTag Tag);

	void BroadcastOnGunnerActionPropertyAdded(const FGunnerActionProperty& NewProperty) const;
	void BroadcastOnGunnerActionPropertyRemoved(const FGunnerActionProperty& RemovedProperty) const;

	void BindOnGunnerActionPropertyAdded(const FGameplayTag& Tag, FOnGunnerActionPropertyCountChangedSignature&& Delegate);
	void BindOnGunnerActionPropertyRemoved(const FGameplayTag& Tag, FOnGunnerActionPropertyCountChangedSignature&& Delegate);


	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FGunnerActionProperty, FGunnerActionPropertyArray>(Items, DeltaParms, *this);
	}

private:
	TMap<FGameplayTag, FOnGunnerActionPropertyCountChangedSignature> OnGunnerActionPropertyAddedDelegates;
	TMap<FGameplayTag, FOnGunnerActionPropertyCountChangedSignature> OnGunnerActionPropertyRemovedDelegates;
};

template <>
struct TStructOpsTypeTraits<FGunnerActionPropertyArray> : public TStructOpsTypeTraitsBase2<FGunnerActionPropertyArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
