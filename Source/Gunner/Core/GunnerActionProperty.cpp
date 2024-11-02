// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionProperty.h"

#include "Gunner/Gunner.h"
#include "Net/UnrealNetwork.h"

void UGunnerActionProperty::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGunnerActionProperty, Tag);
	DOREPLIFETIME_CONDITION_NOTIFY(UGunnerActionProperty, RealValue, COND_None, REPNOTIFY_Always);
}

void UGunnerActionProperty::MakePropertyDirty()
{
	UE_LOG(LogGunner, Warning, TEXT("UGunnerActionProperty::MakePropertyDirty: %f"), GetWorld()->GetTimeSeconds());
	float AdditiveOperand = 0.0f;
	float MultiplicativeOperand = 1.0f;
	float DivisiveOperand = 1.0f;

	float OverrideOperand = 0.0f;
	bool bShouldOverride = false;


	for (const FGunnerActionPropertyOperation& PropertyOperation : Operations)
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

	AActor* ActorOwner = Cast<AActor>(GetOuter());
	bool bHasAuthority = ActorOwner->HasAuthority();

	if (bShouldOverride)
	{
		Value = OverrideOperand;
	}
	else
	{
		Value = ((RealValue + AdditiveOperand) * MultiplicativeOperand) / DivisiveOperand;
	}

	if (bHasAuthority)
	{
		RealValue = Value;
		Operations.Empty();
	}
}

void UGunnerActionProperty::PostNetReceive()
{
	UObject::PostNetReceive();
	UE_LOG(LogGunner, Warning, TEXT("UGunnerActionProperty::PostNetRecieve: %f"), GetWorld()->GetTimeSeconds());
}

void UGunnerActionProperty::OnRep_RealValue(float OldValue)
{
	UE_LOG(LogGunner, Warning, TEXT("UGunnerActionProperty::OnRep_RealValue: %f"), GetWorld()->GetTimeSeconds());
	MakePropertyDirty();
}
