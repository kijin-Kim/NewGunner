// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "NexusProperty.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNexusPropertyChangedSignature, float, OldValue, float, NewValue);


UENUM(BlueprintType)
enum class ENexusPropertyOperator : uint8
{
	Add,
	Subtract,
	Multiply,
	Divide,
	Override,
};

UENUM(BlueprintType)
enum class ENexusPropertyCalculationType : uint8
{
	None, 
	Direct,
	FromOutside,
	PropertyBased,
};

USTRUCT()
struct FNexusPropertyOperationHandle
{
	GENERATED_BODY()

	FNexusPropertyOperationHandle()
		: Handle(INDEX_NONE)
	{
	}

	void GenerateNewHandle();
	bool IsValid() const { return Handle != INDEX_NONE; }
	bool operator==(const FNexusPropertyOperationHandle& Other) const = default;
	FString ToString() const { return FString::Printf(TEXT("%d"), Handle); }

	friend uint32 GetTypeHash(const FNexusPropertyOperationHandle& DefHandle) { return GetTypeHash(DefHandle.Handle); }

private:
	UPROPERTY()
	int32 Handle;
};


USTRUCT()
struct FNexusPropertyOperation
{
	GENERATED_BODY()
	FNexusPropertyOperation()
		: Operand(0.0f)
		  , Operator(ENexusPropertyOperator::Add)
	{
		Handle.GenerateNewHandle();
	}

	FNexusPropertyOperation(float InOperand, ENexusPropertyOperator InOperator)
		: Operand(InOperand)
		  , Operator(InOperator)
	{
		Handle.GenerateNewHandle();
	}

public:
	float Operand;
	ENexusPropertyOperator Operator;
	FNexusPropertyOperationHandle Handle;
};

/**
 * 
 */

UCLASS(BlueprintType)
class NEXUSACTION_API UNexusProperty : public UObject
{
	GENERATED_BODY()

public:
	virtual bool IsNameStableForNetworking() const override { return true; }
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	bool operator==(const UNexusProperty* Other) const { return Tag == Other->Tag; }
	bool operator!=(const UNexusProperty* Other) const { return !(*this == Other); }

	void Tick();
	
	void SetStaticValue(float NewValue);
	void SetDynamicValue(float NewValue);
	void SetTag(FGameplayTag InTag);
	float GetStaticValue() const { return StaticValue; }
	float GetDynamicValue() const { return DynamicValue; }
	FGameplayTag GetTag() const { return Tag; }

	void AddStaticOperation(const FNexusPropertyOperation& Operation);
	void AddDynamicOperation(const FNexusPropertyOperation& Operation);
	void RemoveOperationByHandle(const FNexusPropertyOperationHandle& OperationHandle);

private:
	void Evaluate();
	void EvaluateOperations(const TArray<FNexusPropertyOperation>& PropertyOperations, float& TargetValue);

	UFUNCTION()
	void OnRep_DynamicValue(float OldValue);

public:
	UPROPERTY(BlueprintAssignable)
	FOnNexusPropertyChangedSignature OnChangedDelegate;

private:
	UPROPERTY(Replicated, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	FGameplayTag Tag;

	UPROPERTY(Replicated, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	float StaticValue;
	UPROPERTY(ReplicatedUsing=OnRep_DynamicValue, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	float DynamicValue;

	TArray<FNexusPropertyOperation> StaticOperations;
	TArray<FNexusPropertyOperation> DynamicOperations;

	bool bIsDirty = false;
};
