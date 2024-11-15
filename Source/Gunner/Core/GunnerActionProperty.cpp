// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionProperty.h"

#include "Gunner/Gunner.h"
#include "Net/UnrealNetwork.h"

void UGunnerActionProperty::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGunnerActionProperty, Tag);
	DOREPLIFETIME(UGunnerActionProperty, StaticValue);
	DOREPLIFETIME(UGunnerActionProperty, DynamicValue);
}

void UGunnerActionProperty::MakePropertyDirty()
{
	Evaluate(StaticOperations, StaticValue);
	Evaluate(DynamicOperations, DynamicValue);
	GR_LOG_SUB(LogGunner, Display, TEXT("Property [%s] StaticValue: %f, DynamicValue: %f, Time: %f"), *Tag.ToString(), StaticValue, DynamicValue, GetWorld()->GetTimeSeconds());
}

void UGunnerActionProperty::Evaluate(const TArray<FGunnerActionPropertyOperation>& PropertyOperations, float& TargetValue)
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

void UGunnerActionProperty::PreNetReceive()
{
	UObject::PreNetReceive();
	GR_LOG_SUB(LogGunner, Warning, TEXT( "" ));
}

void UGunnerActionProperty::PostNetReceive()
{
	UObject::PostNetReceive();
	GR_LOG_SUB(LogGunner, Warning, TEXT( "" ));
	AActor* ActorOwner = Cast<AActor>(GetOuter());
	check(ActorOwner);
	if (ActorOwner->GetLocalRole() != ROLE_SimulatedProxy)
	{
		MakePropertyDirty();
	}
}
