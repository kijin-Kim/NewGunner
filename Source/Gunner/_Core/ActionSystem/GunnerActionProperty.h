// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Gunner/_Core/ReplicatableObject.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "UObject/Object.h"
#include "GunnerActionProperty.generated.h"


DECLARE_DELEGATE_TwoParams(FOnGunnerActionPropertyValueChangedSignature, float OldValue, float NewValue);
DECLARE_DELEGATE_OneParam(FOnGunnerActionPropertyCountChangedSignature, UGunnerActionProperty* NewProperty);

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

public:
	float Operand;
	EGunnerActionPropertyOperator Operator;
	FGunnerActionPropertyOperationHandle Handle;
};

/**
 * 
 */

UCLASS(BlueprintType)
class GUNNER_API UGunnerActionProperty : public UReplicatableObject
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	bool operator==(const UGunnerActionProperty* Other) const { return Tag == Other->Tag; }
	bool operator!=(const UGunnerActionProperty* Other) const { return !(*this == Other); }

	void Tick();


	void SetStaticValue(float NewValue);
	void SetDynamicValue(float NewValue);
	void SetTag(FGameplayTag InTag);
	float GetStaticValue() const { return StaticValue; }
	float GetDynamicValue() const { return DynamicValue; }
	FGameplayTag GetTag() const { return Tag; }

	void AddStaticOperation(const FGunnerActionPropertyOperation& Operation);
	void AddDynamicOperation(const FGunnerActionPropertyOperation& Operation);
	void RemoveOperationByHandle(const FGunnerActionPropertyOperationHandle& OperationHandle);
	

private:
	void Evaluate();
	void EvaluateOperations(const TArray<FGunnerActionPropertyOperation>& PropertyOperations, float& TargetValue);

public:
	FOnGunnerActionPropertyValueChangedSignature OnGunnerActionPropertyValueChangedDelegate;

private:
	UPROPERTY(Replicated, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	FGameplayTag Tag;

	UPROPERTY(Replicated, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	float StaticValue;
	UPROPERTY(Replicated, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	float DynamicValue;

	TArray<FGunnerActionPropertyOperation> StaticOperations;
	TArray<FGunnerActionPropertyOperation> DynamicOperations;
	
	bool bIsDirty = false;
};
