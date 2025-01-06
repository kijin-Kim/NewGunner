// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "UObject/Object.h"
#include "GunnerActionProperty.generated.h"


DECLARE_DELEGATE_TwoParams(FOnGunnerActionPropertyValueChangedSignature, float OldValue, float NewValue);
DECLARE_DELEGATE_OneParam(FOnGunnerActionPropertyCountChangedSignature, const struct FGunnerActionProperty& NewProperty);

UENUM(BlueprintType)
enum class EGunnerActionPropertyOperator
{
	Add,
	Subtract,
	Multiply,
	Divide,
	Override
};

UENUM(BlueprintType)
enum class EGunnerActionPropertyCalculationType
{
	None,
	Direct,
	FromOutside,
	PropertyBased,
};

USTRUCT()
struct FGunnerActionPropertyOperationHandle
{
	GENERATED_BODY()

	FGunnerActionPropertyOperationHandle()
		: Handle(INDEX_NONE)
	{
	}

	void GenerateNewHandle();
	bool IsValid() const { return Handle != INDEX_NONE; }
	bool operator==(const FGunnerActionPropertyOperationHandle& Other) const = default;
	FString ToString() const { return FString::Printf(TEXT("%d"), Handle); }

	friend uint32 GetTypeHash(const FGunnerActionPropertyOperationHandle& DefHandle) { return ::GetTypeHash(DefHandle.Handle); }

private:
	UPROPERTY()
	int32 Handle;
};


USTRUCT()
struct FGunnerActionPropertyOperation
{
	GENERATED_BODY()
	FGunnerActionPropertyOperation()
		: Operand(0.0f)
		  , Operator(EGunnerActionPropertyOperator::Add)
	{
		Handle.GenerateNewHandle();
	}

	FGunnerActionPropertyOperation(float InOperand, EGunnerActionPropertyOperator InOperator)
		: Operand(InOperand)
		  , Operator(InOperator)
	{
		Handle.GenerateNewHandle();
	}

	void SetOperand(float InOperand);
	void SetOperator(EGunnerActionPropertyOperator InOperator);
	void SetHandle(FGunnerActionPropertyOperationHandle InHandle) { Handle = InHandle; }


	float GetOperand() const { return Operand; }
	EGunnerActionPropertyOperator GetOperator() const { return Operator; }
	FGunnerActionPropertyOperationHandle GetHandle() const { return Handle; }

private:
	float Operand;
	EGunnerActionPropertyOperator Operator;
	FGunnerActionPropertyOperationHandle Handle;
};

/**
 * 
 */


USTRUCT(BlueprintType)
struct FGunnerActionProperty : public FFastArraySerializerItem
{
	GENERATED_USTRUCT_BODY()

	friend struct FGunnerActionPropertyArray;

public:
	bool operator==(const FGunnerActionProperty& Other) const
	{
		return Tag == Other.Tag;
	}

	bool operator!=(const FGunnerActionProperty& Other) const
	{
		return !(*this == Other);
	}

	void PostReplicatedAdd(const FGunnerActionPropertyArray& InArraySerializer);
	void PreReplicatedRemove(const FGunnerActionPropertyArray& InArraySerializer);
	void PostReplicatedChange(const FGunnerActionPropertyArray& InArraySerializer);

private:
	void Evaluate();
	void EvaluateOperations(const TArray<FGunnerActionPropertyOperation>& PropertyOperations, float& TargetValue);

public:
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Tag;
	UPROPERTY()
	float StaticValue;
	UPROPERTY(NotReplicated, BlueprintReadOnly)
	float DynamicValue;

	TArray<FGunnerActionPropertyOperation> StaticOperations;
	TArray<FGunnerActionPropertyOperation> DynamicOperations;

	FOnGunnerActionPropertyValueChangedSignature OnGunnerActionPropertyValueChangedDelegate;

	bool bIsDirty = false;
};

USTRUCT()
struct FGunnerActionPropertyArray : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	void AuthAdd(const FGunnerActionProperty& Property);
	void AuthRemove(FGameplayTag Tag);
	void AuthRemoveAll();

	void AddStaticOperation(FGameplayTag Tag, FGunnerActionPropertyOperation Operation);
	void AddDynamicOperation(FGameplayTag Tag, FGunnerActionPropertyOperation Operation);
	void RemoveOperationByHandle(FGameplayTag Tag, const FGunnerActionPropertyOperationHandle& OperationHandle);
	FGunnerActionPropertyOperation* FindOperationByHandle(FGunnerActionPropertyOperationHandle OperationHandle);


	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);
	void Tick();

public:
	UPROPERTY()
	TArray<FGunnerActionProperty> Items;
};

template <>
struct TStructOpsTypeTraits<FGunnerActionPropertyArray> : public TStructOpsTypeTraitsBase2<FGunnerActionPropertyArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
