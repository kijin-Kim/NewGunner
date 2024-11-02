// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ActionSystem/GunnerActionSideEffectDefinitionHandle.h"
#include "UObject/Object.h"
#include "GunnerActionProperty.generated.h"


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
	FGunnerActionSideEffectDefinitionHandle SideEffectDefinitionHandle;
};


USTRUCT()
struct FGunnerActionPropertyModifier
{
	GENERATED_BODY()

public:
	void Add(float Operand)
	{
		AdditiveOperand += Operand;
	}

	void Subtract(float Operand)
	{
		AdditiveOperand -= Operand;
	}

	void Multiply(float Operand)
	{
		MultiplicativeOperand += Operand;
	}

	void Divide(float Operand)
	{
		DivisiveOperand += Operand;
	}

	void Override(float NewValue)
	{
		OverrideOperand = NewValue;
		bShouldOverride = true;
	}

	void Evaluate()
	{
		if (FMath::IsNearlyZero(MultiplicativeOperand))
		{
			MultiplicativeOperand = 1.0f;
		}

		//float& TargetValue = bHasAuthority ? StaticValue : DynamicValue;
		float& TargetValue = StaticValue;
		const float OldValue = TargetValue;

		if (bShouldOverride)
		{
			TargetValue = OverrideOperand;
		}
		else
		{
			TargetValue = ((TargetValue + AdditiveOperand) * MultiplicativeOperand) / DivisiveOperand;
		}
	}

public:
	float MultiplicativeOperand = 1.0f;
	float AdditiveOperand = 0.0f;
	float DivisiveOperand = 1.0f;

	float OverrideOperand = 0.0f;
	bool bShouldOverride = false;

	float StaticValue = 0.0f;
};

/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerActionProperty : public UObject
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override { return true; }
	void MakePropertyDirty();
	virtual void PostNetReceive() override;
	
	UFUNCTION()
	void OnRep_RealValue(float OldValue);

public:
	UPROPERTY(Replicated)
	FGameplayTag Tag;
	UPROPERTY(ReplicatedUsing = OnRep_RealValue)
	float RealValue;
	float Value;
	TArray<FGunnerActionPropertyOperation> Operations;
};
