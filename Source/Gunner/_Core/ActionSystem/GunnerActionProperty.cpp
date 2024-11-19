// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionProperty.h"

void FGunnerActionProperty::MarkPropertyDirty()
{
	float OldValue = DynamicValue;
	Evaluate(StaticOperations, StaticValue);
	Evaluate(DynamicOperations, DynamicValue);
	OnGunnerActionPropertyValueChangedDelegate.ExecuteIfBound(OldValue, DynamicValue);
}

void FGunnerActionProperty::Evaluate(const TArray<FGunnerActionPropertyOperation>& PropertyOperations, float& TargetValue)
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

void FGunnerActionProperty::PostReplicatedAdd(const FGunnerActionPropertyArray& InArraySerializer)
{
	InArraySerializer.BroadcastOnGunnerActionPropertyAdded(*this);
	MarkPropertyDirty();
}

void FGunnerActionProperty::PreReplicatedRemove(const FGunnerActionPropertyArray& InArraySerializer)
{
	InArraySerializer.BroadcastOnGunnerActionPropertyRemoved(*this);
}

void FGunnerActionProperty::PostReplicatedChange(const FGunnerActionPropertyArray& InArraySerializer)
{
	MarkPropertyDirty();
}

void FGunnerActionPropertyArray::AuthAdd(const FGunnerActionProperty& Item)
{
	check(Items.Find(Item) == INDEX_NONE);
	const int32 Index = Items.Add(Item);
	MarkItemDirty(Items[Index]);
	BroadcastOnGunnerActionPropertyAdded(Items[Index]);
	Items[Index].MarkPropertyDirty();
}

void FGunnerActionPropertyArray::AuthRemove(FGameplayTag Tag)
{
	if (FGunnerActionProperty* PropertyPtr = Items.FindByPredicate([Tag](const FGunnerActionProperty& Property)
	{
		return Property.Tag == Tag;
	}))
	{
		PropertyPtr->MarkPropertyDirty();
		BroadcastOnGunnerActionPropertyRemoved(*PropertyPtr);
	}

	Items.RemoveAll([Tag](const FGunnerActionProperty& Property)
	{
		return Property.Tag == Tag;
	});
	MarkArrayDirty();
}

void FGunnerActionPropertyArray::BroadcastOnGunnerActionPropertyAdded(const FGunnerActionProperty& NewProperty) const
{
	if (const FOnGunnerActionPropertyCountChangedSignature* Delegate = OnGunnerActionPropertyAddedDelegates.Find(NewProperty.Tag))
	{
		Delegate->ExecuteIfBound(NewProperty);
	}
}

void FGunnerActionPropertyArray::BroadcastOnGunnerActionPropertyRemoved(const FGunnerActionProperty& RemovedProperty) const
{
	if (const FOnGunnerActionPropertyCountChangedSignature* Delegate = OnGunnerActionPropertyRemovedDelegates.Find(RemovedProperty.Tag))
	{
		Delegate->ExecuteIfBound(RemovedProperty);
	}
}

void FGunnerActionPropertyArray::BindOnGunnerActionPropertyAdded(const FGameplayTag& Tag, FOnGunnerActionPropertyCountChangedSignature&& Delegate)
{
	OnGunnerActionPropertyAddedDelegates.Add(Tag, MoveTemp(Delegate));
}

void FGunnerActionPropertyArray::BindOnGunnerActionPropertyRemoved(const FGameplayTag& Tag, FOnGunnerActionPropertyCountChangedSignature&& Delegate)
{
	OnGunnerActionPropertyRemovedDelegates.Add(Tag, MoveTemp(Delegate));
}
