// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionSideEffect.h"

#include "GunnerActionComponent.h"
#include "Gunner/Gunner.h"


void UGunnerActionSideEffect::OnApplied(FGunnerActionNetPredictionHandle PredictionHandle, bool bHasAuthority)
{
	GR_LOG_SUB(LogGunnerSideEffect, Verbose, TEXT("SideEffect [%s] 적용"), *GetName());
	RemainingDuration = Duration;
	ApplyAllModifiers(PredictionHandle, bHasAuthority);
}

void UGunnerActionSideEffect::OnTick(float DeltaTime, bool bHasAuthority)
{
	if (DurationType == ESideEffectDurationType::Duration)
	{
		RemainingDuration -= DeltaTime;
	}

	if (Interval <= 0.0f)
	{
		return;
	}

	float TimePaseedBeforeDuration = DeltaTime;
	if (RemainingDuration < 0.0f)
	{
		TimePaseedBeforeDuration += RemainingDuration;
	}

	ElapsedTime += TimePaseedBeforeDuration;
	if (ElapsedTime < Interval)
	{
		return;
	}
	int32 ApplyCount = ElapsedTime / Interval;
	ElapsedTime -= ApplyCount * Interval;

	for (int32 i = 0; i < ApplyCount; ++i)
	{
		ApplyAllModifiers(FGunnerActionNetPredictionHandle(), bHasAuthority);
	}
}

void UGunnerActionSideEffect::OnRemoved()
{
	GR_LOG_SUB(LogGunnerSideEffect, Verbose, TEXT("SideEffect [%s] 삭제"), *GetName());
	AActor* ActorOwner = Cast<AActor>(GetOuter());
	UGunnerActionComponent* ActionComponent = UGunnerActionComponent::GetActionComponentFromActor(ActorOwner);
	check(ActionComponent);
	for (FGunnerPropertyModifier& Modifier : Modifiers)
	{
		for (FGunnerActionPropertyOperationHandle OperationHandle : OperationHandles)
		{
			ActionComponent->RemoveOperationByHandle(Modifier.PropertyTag, OperationHandle);
		}
	}
}

void UGunnerActionSideEffect::SetInjectedValue(FGameplayTag Tag, float Value)
{
	InjectedValues.FindOrAdd(Tag, Value);
}

void UGunnerActionSideEffect::ApplyModifier(const FGunnerPropertyModifier& Modifier, FGunnerActionNetPredictionHandle PredictionHandle, bool bHasAuthority)
{
	AActor* ActorOwner = Cast<AActor>(GetOuter());
	UGunnerActionComponent* ActionComponent = UGunnerActionComponent::GetActionComponentFromActor(ActorOwner);;
	check(ActionComponent);

	if (DurationType != ESideEffectDurationType::Instant && Interval > 0.0f
		&& !bHasAuthority)
	{
		return;
	}


	UGunnerActionProperty* Property = ActionComponent->GetProperty(Modifier.PropertyTag);
	if (!Property)
	{
		return;
	}

	if (Modifier.CalculationType == EGunnerActionPropertyCalculationType::None)
	{
		return;
	}

	float DesiredValue = 0.0f;
	if (Modifier.CalculationType == EGunnerActionPropertyCalculationType::Direct)
	{
		DesiredValue = Modifier.DirectValue;
	}

	if (Modifier.CalculationType == EGunnerActionPropertyCalculationType::PropertyBased)
	{
		DesiredValue = UGunnerActionComponent::GetPropertyValueFromActor(ActorOwner, Modifier.BaseProperty);
	}

	if (Modifier.CalculationType == EGunnerActionPropertyCalculationType::FromOutside)
	{
		const float* InjectedValuePtr = InjectedValues.Find(Modifier.InjectedValueTag);
		DesiredValue = InjectedValuePtr ? *InjectedValuePtr : 0.0f;
	}


	if ((DurationType != ESideEffectDurationType::Instant && Interval <= 0.0f)
		|| (PredictionHandle.IsValid() && !ActorOwner->HasAuthority()))
	{
		FGunnerActionPropertyOperation NewOperation{DesiredValue, Modifier.Operator};
		OperationHandles.Add(NewOperation.Handle);
		ActionComponent->AddDynamicOperation(Modifier.PropertyTag, NewOperation);
	}
	else
	{
		FGunnerActionPropertyOperation NewOperation{DesiredValue, Modifier.Operator};
		OperationHandles.Add(NewOperation.Handle);
		ActionComponent->AddStaticOperation(Modifier.PropertyTag, NewOperation);
	}
}

void UGunnerActionSideEffect::ApplyAllModifiers(FGunnerActionNetPredictionHandle PredictionHandle, bool bHasAuthority)
{
	for (const FGunnerPropertyModifier& Modifier : Modifiers)
	{
		ApplyModifier(Modifier, PredictionHandle, bHasAuthority);
	}
}
