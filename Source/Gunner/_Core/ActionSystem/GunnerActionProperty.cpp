// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionProperty.h"

#include "GunnerActionSideEffect.h"
#include "Gunner/Gunner.h"
#include "Net/UnrealNetwork.h"

void FGunnerActionPropertyOperationHandle::GenerateNewHandle()
{
	static int32 HandleCounter = 1;
	Handle = HandleCounter++;
}


void UGunnerActionProperty::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGunnerActionProperty, Tag);
	DOREPLIFETIME(UGunnerActionProperty, StaticValue);
	DOREPLIFETIME_CONDITION(UGunnerActionProperty, DynamicValue, COND_SimulatedOnly);
}

void UGunnerActionProperty::Tick()
{
	if (bIsDirty)
	{
		Evaluate();
	}
}

void UGunnerActionProperty::SetStaticValue(float NewValue)
{
	StaticValue = NewValue;
	bIsDirty = true;
}

void UGunnerActionProperty::SetDynamicValue(float NewValue)
{
	DynamicValue = NewValue;
	bIsDirty = true;
}

void UGunnerActionProperty::SetTag(FGameplayTag InTag)
{
	Tag = InTag;
}

void UGunnerActionProperty::AddStaticOperation(const FGunnerActionPropertyOperation& Operation)
{
	StaticOperations.Add(Operation);
	bIsDirty = true;
}

void UGunnerActionProperty::AddDynamicOperation(const FGunnerActionPropertyOperation& Operation)
{
	DynamicOperations.Add(Operation);
	bIsDirty = true;
}

void UGunnerActionProperty::RemoveOperationByHandle(const FGunnerActionPropertyOperationHandle& OperationHandle)
{
	StaticOperations.RemoveAll([OperationHandle](const FGunnerActionPropertyOperation& Operation)
	{
		return Operation.Handle == OperationHandle;
	});
	DynamicOperations.RemoveAll([OperationHandle](const FGunnerActionPropertyOperation& Operation)
	{
		return Operation.Handle == OperationHandle;
	});
	bIsDirty = true;
}

void UGunnerActionProperty::Evaluate()
{
	float OldValue = DynamicValue;
	EvaluateOperations(StaticOperations, StaticValue);
	StaticOperations.Empty();
	EvaluateOperations(DynamicOperations, DynamicValue);
	bIsDirty = false;
	if (OldValue != DynamicValue)
	{
		GR_LOG_SUB(LogGunnerProperty, Verbose, TEXT("Property [%s] 값 변경 %f -> %f"), *Tag.ToString(), OldValue, DynamicValue);
		OnGunnerActionPropertyValueChangedDelegate.Broadcast(OldValue, DynamicValue);
	}
}


void UGunnerActionProperty::EvaluateOperations(const TArray<FGunnerActionPropertyOperation>& PropertyOperations, float& TargetValue)
{
	float AdditiveOperand = 0.0f;
	float MultiplicativeOperand = 1.0f;
	float DivisiveOperand = 1.0f;

	float OverrideOperand = 0.0f;
	bool bShouldOverride = false;


	for (const FGunnerActionPropertyOperation& PropertyOperation : PropertyOperations)
	{
		switch (PropertyOperation.Operator)
		{
		case EGunnerActionPropertyOperator::Add:
			AdditiveOperand += PropertyOperation.Operand;
			break;
		case EGunnerActionPropertyOperator::Subtract:
			AdditiveOperand -= PropertyOperation.Operand;
			break;
		case EGunnerActionPropertyOperator::Multiply:
			MultiplicativeOperand += PropertyOperation.Operand;
			break;
		case EGunnerActionPropertyOperator::Divide:
			DivisiveOperand += PropertyOperation.Operand;
			break;
		case EGunnerActionPropertyOperator::Override:
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

void UGunnerActionProperty::OnRep_DynamicValue(float OldValue)
{
	OnGunnerActionPropertyValueChangedDelegate.Broadcast(OldValue, DynamicValue);
}
