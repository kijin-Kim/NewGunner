// Fill out your copyright notice in the Description page of Project Settings.


#include "SideEffect/NexusSideEffect.h"

#include "NexusActionComponent.h"
#include "NexusLog.h"


void UNexusSideEffect::OnApplied(FNexusPredictionTag PredictionTag, bool bHasAuthority)
{
	NX_VLOG_SUB(Cast<AActor>(GetOuter()), LogNexusSideEffect, Log, TEXT("사이드 이펙트 [%s] 적용"), *GetName());
	RemainingDuration = Duration;
	ApplyAllModifiers(PredictionTag, bHasAuthority);
}

void UNexusSideEffect::OnTick(float DeltaTime, bool bHasAuthority)
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
		ApplyAllModifiers(FNexusPredictionTag(), bHasAuthority);
	}
}

void UNexusSideEffect::OnRemoved()
{
	
	AActor* ActorOwner = Cast<AActor>(GetOuter());
	NX_VLOG_SUB(ActorOwner, LogNexusSideEffect, Log, TEXT("사이드 이펙트 [%s] 삭제"), *GetName());
	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(ActorOwner);
	check(ActionComponent);
	for (FNexusPropertyMod& Modifier : Modifiers)
	{
		for (FNexusPropertyOperationHandle OperationHandle : OperationHandles)
		{
			ActionComponent->RemoveOperationByHandle(Modifier.PropertyTag, OperationHandle);
		}
	}
}

void UNexusSideEffect::SetInjectedValue(FGameplayTag Tag, float Value)
{
	InjectedValues.FindOrAdd(Tag, Value);
}

void UNexusSideEffect::ApplyModifier(const FNexusPropertyMod& Modifier, FNexusPredictionTag PredictionTag, bool bHasAuthority)
{
	AActor* ActorOwner = Cast<AActor>(GetOuter());
	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(ActorOwner);;
	check(ActionComponent);

	if (DurationType != ESideEffectDurationType::Instant && Interval > 0.0f
		&& !bHasAuthority)
	{
		return;
	}


	UNexusProperty* Property = ActionComponent->GetProperty(Modifier.PropertyTag);
	if (!Property)
	{
		return;
	}

	if (Modifier.CalculationType == ENexusPropertyCalculationType::None)
	{
		return;
	}

	float DesiredValue = 0.0f;
	if (Modifier.CalculationType == ENexusPropertyCalculationType::Direct)
	{
		DesiredValue = Modifier.DirectValue;
	}

	if (Modifier.CalculationType == ENexusPropertyCalculationType::PropertyBased)
	{
		DesiredValue = UNexusActionComponent::GetPropertyValueFromActor(ActorOwner, Modifier.BaseProperty);
	}

	if (Modifier.CalculationType == ENexusPropertyCalculationType::FromOutside)
	{
		const float* InjectedValuePtr = InjectedValues.Find(Modifier.InjectedValueTag);
		DesiredValue = InjectedValuePtr ? *InjectedValuePtr : 0.0f;
	}


	if ((DurationType != ESideEffectDurationType::Instant && Interval <= 0.0f)
		|| (PredictionTag.IsValid() && !ActorOwner->HasAuthority()))
	{
		FNexusPropertyOperation NewOperation{DesiredValue, Modifier.Operator};
		OperationHandles.Add(NewOperation.Handle);
		ActionComponent->AddDynamicOperation(Modifier.PropertyTag, NewOperation);
	}
	else
	{
		FNexusPropertyOperation NewOperation{DesiredValue, Modifier.Operator};
		OperationHandles.Add(NewOperation.Handle);
		ActionComponent->AddStaticOperation(Modifier.PropertyTag, NewOperation);
	}
}

void UNexusSideEffect::ApplyAllModifiers(FNexusPredictionTag PredictionTag, bool bHasAuthority)
{
	for (const FNexusPropertyMod& Modifier : Modifiers)
	{
		ApplyModifier(Modifier, PredictionTag, bHasAuthority);
	}
}
