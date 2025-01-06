// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionProperty.h"

#include "GunnerActionSideEffect.h"
#include "GunnerActionSideEffectDefinition.h"

void FGunnerActionPropertyOperationHandle::GenerateNewHandle()
{
	static int32 HandleCounter = 1;
	Handle = HandleCounter++;
}

void FGunnerActionPropertyOperation::SetOperand(float InOperand)
{
	Operand = InOperand;
}

void FGunnerActionPropertyOperation::SetOperator(EGunnerActionPropertyOperator InOperator)
{
	Operator = InOperator;
}


void FGunnerActionProperty::PostReplicatedAdd(const FGunnerActionPropertyArray& InArraySerializer)
{
	bIsDirty = true;
}

void FGunnerActionProperty::PreReplicatedRemove(const FGunnerActionPropertyArray& InArraySerializer)
{
	bIsDirty = true;
}

void FGunnerActionProperty::PostReplicatedChange(const FGunnerActionPropertyArray& InArraySerializer)
{
	bIsDirty = true;
}

void FGunnerActionProperty::Evaluate()
{
	float OldValue = DynamicValue;
	EvaluateOperations(StaticOperations, StaticValue);
	StaticOperations.Empty();
	EvaluateOperations(DynamicOperations, DynamicValue);
	bIsDirty = false;
	OnGunnerActionPropertyValueChangedDelegate.ExecuteIfBound(OldValue, DynamicValue);
}

void FGunnerActionProperty::EvaluateOperations(const TArray<FGunnerActionPropertyOperation>& PropertyOperations, float& TargetValue)
{
	float AdditiveOperand = 0.0f;
	float MultiplicativeOperand = 1.0f;
	float DivisiveOperand = 1.0f;

	float OverrideOperand = 0.0f;
	bool bShouldOverride = false;


	for (const FGunnerActionPropertyOperation& PropertyOperation : PropertyOperations)
	{
		switch (PropertyOperation.GetOperator())
		{
		case EGunnerActionPropertyOperator::Add:
			AdditiveOperand += PropertyOperation.GetOperand();
			break;
		case EGunnerActionPropertyOperator::Subtract:
			AdditiveOperand -= PropertyOperation.GetOperand();
			break;
		case EGunnerActionPropertyOperator::Multiply:
			MultiplicativeOperand += PropertyOperation.GetOperand();
			break;
		case EGunnerActionPropertyOperator::Divide:
			DivisiveOperand += PropertyOperation.GetOperand();
			break;
		case EGunnerActionPropertyOperator::Override:
			OverrideOperand = PropertyOperation.GetOperand();
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


void FGunnerActionPropertyArray::AuthAdd(const FGunnerActionProperty& Item)
{
	check(Items.Find(Item) == INDEX_NONE);
	const int32 Index = Items.Add(Item);
	Items[Index].bIsDirty = true;
	MarkItemDirty(Items[Index]);
}

void FGunnerActionPropertyArray::AuthRemove(FGameplayTag Tag)
{
	if (FGunnerActionProperty* PropertyPtr = Items.FindByPredicate([Tag](const FGunnerActionProperty& Property)
	{
		return Property.Tag == Tag;
	}))
	{
		PropertyPtr->bIsDirty = true;
	}

	Items.RemoveAll([Tag](const FGunnerActionProperty& Property)
	{
		return Property.Tag == Tag;
	});
	MarkArrayDirty();
}

void FGunnerActionPropertyArray::AuthRemoveAll()
{
	for (FGunnerActionProperty& Property : Items)
	{
		Property.bIsDirty = true;
	}
	Items.Empty();
	MarkArrayDirty();
}

void FGunnerActionPropertyArray::AddStaticOperation(FGameplayTag Tag, FGunnerActionPropertyOperation Operation)
{
	if (FGunnerActionProperty* PropertyPtr = Items.FindByPredicate([Tag](const FGunnerActionProperty& Property)
	{
		return Property.Tag == Tag;
	}))
	{
		PropertyPtr->StaticOperations.Add(Operation);
		PropertyPtr->bIsDirty = true;
		MarkItemDirty(*PropertyPtr);
	}
}

void FGunnerActionPropertyArray::AddDynamicOperation(FGameplayTag Tag, FGunnerActionPropertyOperation Operation)
{
	if (FGunnerActionProperty* PropertyPtr = Items.FindByPredicate([Tag](const FGunnerActionProperty& Property)
	{
		return Property.Tag == Tag;
	}))
	{
		PropertyPtr->DynamicOperations.Add(Operation);
		PropertyPtr->bIsDirty = true;
		MarkItemDirty(*PropertyPtr);
	}
}


void FGunnerActionPropertyArray::RemoveOperationByHandle(FGameplayTag Tag, const FGunnerActionPropertyOperationHandle& OperationHandle)
{
	if (FGunnerActionProperty* PropertyPtr = Items.FindByPredicate([Tag](const FGunnerActionProperty& Property)
	{
		return Property.Tag == Tag;
	}))
	{
		PropertyPtr->StaticOperations.RemoveAll([OperationHandle](const FGunnerActionPropertyOperation& Operation)
		{
			return Operation.GetHandle() == OperationHandle;
		});
		PropertyPtr->DynamicOperations.RemoveAll([OperationHandle](const FGunnerActionPropertyOperation& Operation)
		{
			return Operation.GetHandle() == OperationHandle;
		});
		PropertyPtr->bIsDirty = true;
		MarkItemDirty(*PropertyPtr);
	}
}

FGunnerActionPropertyOperation* FGunnerActionPropertyArray::FindOperationByHandle(FGunnerActionPropertyOperationHandle OperationHandle)
{
	for (FGunnerActionProperty& Property : Items)
	{
		if (FGunnerActionPropertyOperation* OperationPtr = Property.StaticOperations.FindByPredicate([OperationHandle](const FGunnerActionPropertyOperation& Operation)
		{
			return Operation.GetHandle() == OperationHandle;
		}))
		{
			return OperationPtr;
		}

		if (FGunnerActionPropertyOperation* OperationPtr = Property.DynamicOperations.FindByPredicate([OperationHandle](const FGunnerActionPropertyOperation& Operation)
		{
			return Operation.GetHandle() == OperationHandle;
		}))
		{
			return OperationPtr;
		}
	}

	return nullptr;
}

bool FGunnerActionPropertyArray::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FGunnerActionProperty, FGunnerActionPropertyArray>(Items, DeltaParms, *this);
}

void FGunnerActionPropertyArray::Tick()
{
	for (FGunnerActionProperty& Property : Items)
	{
		if (Property.bIsDirty)
		{
			Property.Evaluate();
			MarkItemDirty(Property);
		}
	}
}
