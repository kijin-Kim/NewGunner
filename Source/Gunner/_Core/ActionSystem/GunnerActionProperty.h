// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Gunner/_Core/ActionSystem/GunnerActionSideEffectDefinitionHandle.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "UObject/Object.h"
#include "GunnerActionProperty.generated.h"


struct FGunnerActionSideEffectDefinition;
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

UENUM()
enum class EGunnerActionPropertyCalculationType
{
	None,
	Direct,
	FromOutside,
	PropertyBased,
};


USTRUCT()
struct FGunnerActionPropertyOperation
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EGunnerActionPropertyCalculationType CalculationType;
	UPROPERTY(EditAnywhere)
	EGunnerActionPropertyOperator Operator;
	
	UPROPERTY(EditAnywhere, meta = (EditCondition = "CalculationType == EGunnerActionPropertyCalculationType::Direct"))
	float Operand;
	UPROPERTY(EditAnywhere, meta = (EditCondition = "CalculationType == EGunnerActionPropertyCalculationType::FromOutside"))
	FGameplayTag OutsideSource;
	UPROPERTY(EditAnywhere, meta = (EditCondition = "CalculationType == EGunnerActionPropertyCalculationType::PropertyBased"))
	FGameplayTag BaseProperty;


	UPROPERTY(EditAnywhere)
	float Duration;

};

USTRUCT()
struct FGunnerActionPropertyInternalOperation
{
	GENERATED_BODY()
	float Operand;
	EGunnerActionPropertyOperator Operator;
	FGunnerActionSideEffectDefinitionHandle SideEffectDefinitionHandle;
};

/**
 * 
 */

struct FGunnerActionPropertyArray;

USTRUCT(BlueprintType)
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
	void Evaluate(const TArray<FGunnerActionPropertyInternalOperation>& PropertyOperations, float& TargetValue);
	void PostReplicatedAdd(const FGunnerActionPropertyArray& InArraySerializer);
	void PreReplicatedRemove(const FGunnerActionPropertyArray& InArraySerializer);
	void PostReplicatedChange(const FGunnerActionPropertyArray& InArraySerializer);


public:
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Tag;
	UPROPERTY()
	float StaticValue;
	UPROPERTY(BlueprintReadOnly)
	float DynamicValue;

	TArray<FGunnerActionPropertyInternalOperation> InternalStaticOperations;
	TArray<FGunnerActionPropertyInternalOperation> InternalDynamicOperations;

	FOnGunnerActionPropertyValueChangedSignature OnGunnerActionPropertyValueChangedDelegate;
};

USTRUCT()
struct FGunnerActionPropertyArray : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TArray<FGunnerActionProperty> Items;

	void OnSideEffectDefinitionAdded(const FGunnerActionSideEffectDefinition& SideEffectDefinition, bool bIsPredictingClient);
	void OnSideEffectDefinitionRemoved(FGunnerActionSideEffectDefinitionHandle SideEffectDefinitionHandle);

	void AuthAdd(const FGunnerActionProperty& Property);
	void AuthRemove(FGameplayTag Tag);
	void AuthRemoveAll();

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
