// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionProperty.h"

#include "GunnerActionSideEffect.h"
#include "GunnerActionSideEffectDefinition.h"

void FGunnerActionProperty::MarkPropertyDirty()
{
	float OldValue = DynamicValue;
	Evaluate(InternalStaticOperations, StaticValue);
	Evaluate(InternalDynamicOperations, DynamicValue);


	OnGunnerActionPropertyValueChangedDelegate.ExecuteIfBound(OldValue, DynamicValue);
}

void FGunnerActionProperty::Evaluate(const TArray<FGunnerActionPropertyInternalOperation>& PropertyOperations, float& TargetValue)
{
	float AdditiveOperand = 0.0f;
	float MultiplicativeOperand = 1.0f;
	float DivisiveOperand = 1.0f;

	float OverrideOperand = 0.0f;
	bool bShouldOverride = false;


	for (const FGunnerActionPropertyInternalOperation& PropertyOperation : PropertyOperations)
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

void FGunnerActionPropertyArray::OnSideEffectDefinitionAdded(const FGunnerActionSideEffectDefinition& SideEffectDefinition, bool bIsPredictingClient)
{
	UGunnerActionSideEffect* SideEffect = SideEffectDefinition.SideEffectCDO;
	check(SideEffect);
	FGunnerActionProperty* PropertyPtr = Items.FindByPredicate([SideEffect](const FGunnerActionProperty& Property)
	{
		return Property.Tag == SideEffect->PropertyTag;
	});
	if (!PropertyPtr)
	{
		return;
	}

	if (SideEffect->CalculationType == EGunnerActionPropertyCalculationType::None)
	{
		return;
	}

	float Value = 0.0f;
	if (SideEffect->CalculationType == EGunnerActionPropertyCalculationType::Direct)
	{
		Value = SideEffect->DirectValue;
	}
	else if (SideEffect->CalculationType == EGunnerActionPropertyCalculationType::FromOutside)
	{
		if (const float* FoundValuePtr = SideEffectDefinition.OutsideSourceValues.Find(SideEffect->OutsideSource))
		{
			Value = *FoundValuePtr;
		}
	}
	else if (SideEffect->CalculationType == EGunnerActionPropertyCalculationType::PropertyBased)
	{
		UE_DEBUG_BREAK();
		FGunnerActionProperty* BaseProperty = Items.FindByPredicate([SideEffect](const FGunnerActionProperty& Property)
		{
			return Property.Tag == SideEffect->BaseProperty;
		});
		if (BaseProperty)
		{
			Value = BaseProperty->DynamicValue;
		}
	}


	if (SideEffect->DurationType != ESideEffectDurationType::Instant || bIsPredictingClient)
	{
		PropertyPtr->InternalDynamicOperations.Add({
			.Operand = Value,
			.Operator = SideEffect->Operator,
			.SideEffectDefinitionHandle = SideEffectDefinition.Handle
		});
	}
	else
	{
		PropertyPtr->InternalStaticOperations.Add({
			.Operand = Value,
			.Operator = SideEffect->Operator,
			.SideEffectDefinitionHandle = SideEffectDefinition.Handle
		});
	}
	PropertyPtr->MarkPropertyDirty();

	// ESideEffectDurationType DurationType = ESideEffectDurationType::Instant;
	// float Duration;
	// float Interval;
	//
	//
	// FGameplayTag PropertyTag;
	// EGunnerActionPropertyCalculationType CalculationType = EGunnerActionPropertyCalculationType::None;
	// EGunnerActionPropertyOperator Operator = EGunnerActionPropertyOperator::Add;
	//
	// float DirectValue;
	// FGameplayTag OutsideSource;
	// FGameplayTag BaseProperty;
}

void FGunnerActionPropertyArray::OnSideEffectDefinitionRemoved(FGunnerActionSideEffectDefinitionHandle SideEffectDefinitionHandle)
{
	for (auto& Property : Items)
	{
		Property.InternalStaticOperations.RemoveAll([SideEffectDefinitionHandle](const FGunnerActionPropertyInternalOperation& Operation)
		{
			return Operation.SideEffectDefinitionHandle == SideEffectDefinitionHandle;
		});

		Property.InternalDynamicOperations.RemoveAll([SideEffectDefinitionHandle](const FGunnerActionPropertyInternalOperation& Operation)
		{
			return Operation.SideEffectDefinitionHandle == SideEffectDefinitionHandle;
		});
		Property.MarkPropertyDirty();
		MarkItemDirty(Property);
	}
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
