// Fill out your copyright notice in the Description page of Project Settings.


#include "SideEffect/NexusProperty.h"

#include "SideEffect/NexusSideEffect.h"
#include "NexusLog.h"
#include "Net/UnrealNetwork.h"

void FNexusPropertyOperationHandle::GenerateNewHandle()
{
	static int32 HandleCounter = 1;
	Handle = HandleCounter++;
}


void UNexusProperty::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UNexusProperty, Tag);
	DOREPLIFETIME_CONDITION(UNexusProperty, StaticValue, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UNexusProperty, DynamicValue, COND_SimulatedOnly);
}

bool UNexusProperty::Evaluate()
{
	if (bIsDirty)
	{
		float OldValue = DynamicValue;
		EvaluateOperations(StaticOperations, StaticValue);
		EvaluateOperations(DynamicOperations, DynamicValue);
		bIsDirty = false;
		OnDirtyDelegate.Broadcast(OldValue, DynamicValue);
		return true;
	}

	return false;
}

void UNexusProperty::SetStaticValue(float NewValue)
{
	StaticValue = NewValue;
	bIsDirty = true;
}

void UNexusProperty::SetDynamicValue(float NewValue)
{
	DynamicValue = NewValue;
	bIsDirty = true;
}

void UNexusProperty::SetTag(FGameplayTag InTag)
{
	Tag = InTag;
}

void UNexusProperty::AddStaticOperation(const FNexusPropertyOperation& Operation)
{
	StaticOperations.Add(Operation);
	bIsDirty = true;
}

void UNexusProperty::AddDynamicOperation(const FNexusPropertyOperation& Operation)
{
	DynamicOperations.Add(Operation);
	bIsDirty = true;
}

FNexusPropertyOperationQueryResult UNexusProperty::FindOperationsByHandle(const FNexusPropertyOperationHandle& OperationHandle) const
{
	FNexusPropertyOperationQueryResult Result;
	Result.StaticOperations = StaticOperations.FilterByPredicate([OperationHandle](const FNexusPropertyOperation& Operation)
	{
		return Operation.Handle == OperationHandle;
	});

	Result.DynamicOperations = DynamicOperations.FilterByPredicate([OperationHandle](const FNexusPropertyOperation& Operation)
	{
		return Operation.Handle == OperationHandle;
	});

	return Result;
}

void UNexusProperty::RemoveStaticOperation(const FNexusPropertyOperation& Operation)
{
	StaticOperations.RemoveAll([Operation](const FNexusPropertyOperation& StaticOperation)
	{
		return StaticOperation.Handle == Operation.Handle;
	});
	bIsDirty = true;
}

void UNexusProperty::RemoveDynamicOperation(const FNexusPropertyOperation& Operation)
{
	DynamicOperations.RemoveAll([Operation](const FNexusPropertyOperation& DynamicOperation)
	{
		return DynamicOperation.Handle == Operation.Handle;
	});
	bIsDirty = true;
}

FString UNexusProperty::ToString() const
{
	return FString::Printf(TEXT("Property={Tag=%s, StaticValue=%.2f, DynamicValue=%.2f}"), *Tag.ToString(), StaticValue, DynamicValue);
}


void UNexusProperty::EvaluateOperations(const TArray<FNexusPropertyOperation>& PropertyOperations, float& TargetValue)
{
	float AdditiveOperand = 0.0f;
	float MultiplicativeOperand = 1.0f;
	float DivisiveOperand = 1.0f;

	float OverrideOperand = 0.0f;
	bool bShouldOverride = false;


	for (const FNexusPropertyOperation& PropertyOperation : PropertyOperations)
	{
		switch (PropertyOperation.Operator)
		{
		case ENexusPropertyOperator::Add:
			AdditiveOperand += PropertyOperation.Operand;
			break;
		case ENexusPropertyOperator::Subtract:
			AdditiveOperand -= PropertyOperation.Operand;
			break;
		case ENexusPropertyOperator::Multiply:
			MultiplicativeOperand += PropertyOperation.Operand;
			break;
		case ENexusPropertyOperator::Divide:
			DivisiveOperand += PropertyOperation.Operand;
			break;
		case ENexusPropertyOperator::Override:
			OverrideOperand = PropertyOperation.Operand;
			bShouldOverride = true;
			break;
		}
	}


	if (FMath::IsNearlyZero(MultiplicativeOperand))
	{
		MultiplicativeOperand = 1.0f;
	}


	if (bShouldOverride)
	{
		TargetValue = OverrideOperand;
	}
	else
	{
		TargetValue = ((StaticValue + AdditiveOperand) * MultiplicativeOperand) / DivisiveOperand;
	}
}

void UNexusProperty::OnRep_StaticValue()
{
	bIsDirty = true;
}

void UNexusProperty::OnRep_DynamicValue(float OldValue)
{
	OnDirtyDelegate.Broadcast(OldValue, DynamicValue);
}
