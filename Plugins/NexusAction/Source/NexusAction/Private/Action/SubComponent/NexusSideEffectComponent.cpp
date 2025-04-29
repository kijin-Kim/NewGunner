// Fill out your copyright notice in the Description page of Project Settings.


#include "Action/SubComponent/NexusSideEffectComponent.h"

#include "NexusLog.h"
#include "Action/NexusAction.h"
#include "Action/SubComponent/NexusGameplayTagComponent.h"
#include "Action/SubComponent/NexusPropertyComponent.h"
#include "Net/UnrealNetwork.h"
#include "SideEffect/NexusSideEffect.h"
#include "SideEffect/NexusSideEffectInstance.h"


UNexusSideEffectComponent::UNexusSideEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UNexusSideEffectComponent::Setup(TSharedPtr<FNexusAgentInfo> InAgentInfo)
{
	Super::Setup(InAgentInfo);
	UNexusPropertyComponent* PropertyComponent = GetOwner()->GetComponentByClass<UNexusPropertyComponent>();
	check(PropertyComponent);
	UNexusGameplayTagComponent* GameplayTagComponent = GetOwner()->GetComponentByClass<UNexusGameplayTagComponent>();
	check(GameplayTagComponent);

	SideEffectInstances.Init(PropertyComponent, GameplayTagComponent, GetOwner()->HasAuthority());
}

void UNexusSideEffectComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UNexusSideEffectComponent, SideEffectInstances, COND_OwnerOnly);
}

void UNexusSideEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SideEffectInstances.Tick(DeltaTime);
}

FNexusSideEffectInstanceHandle UNexusSideEffectComponent::ApplySideEffectByDef(const FNexusSideEffectInstanceDef& SideEffectInstanceDef, FNexusPredictionTag PredictionTag, FNexusPredictionEventSignature::FDelegate&& OnPredictionEnded, FNexusPredictionEventSignature::FDelegate&& OnPredictionFailed)
{
	if (!GetOwner()->HasAuthority() && !PredictionTag.IsPredictable())
	{
		NX_VLOG_SUB(GetAgentActor(), LogNexusSideEffect, Error, TEXT("사이드이펙트 실행 오류 (예측 불가): %s; %s"), *SideEffectInstanceDef.ToString(), *PredictionTag.ToString());
		return FNexusSideEffectInstanceHandle();
	}

	FNexusSideEffectInstanceHandle SideEffectInstanceHandle = RegisterAndApplySideEffect(SideEffectInstanceDef);
	if (!GetOwner()->HasAuthority() && PredictionTag.IsPredictable())
	{
		FNexusPredictionEvents::FPredictionEvent& PredictionEvent = FNexusPredictionEvents::GetPredictionEvent(PredictionTag);
		if (OnPredictionEnded.IsBound())
		{
			PredictionEvent.OnPredictionEnded.Add(MoveTemp(OnPredictionEnded));
		}
		if (OnPredictionFailed.IsBound())
		{
			PredictionEvent.OnPredictionFailed.Add(MoveTemp(OnPredictionFailed));
		}

		PredictionEvent.OnPredictionEnded.AddWeakLambda(this, [this,SideEffectInstanceHandle]()
		{
			UnregisterAndRemoveSideEffect(SideEffectInstanceHandle);
		});

		PredictionEvent.OnPredictionFailed.AddWeakLambda(this, [this,SideEffectInstanceHandle]()
		{
			UnregisterAndRemoveSideEffect(SideEffectInstanceHandle);
		});
	}
	return SideEffectInstanceHandle;
}

const FNexusSideEffectInstanceContainer& UNexusSideEffectComponent::GetSideEffectInstances() const
{
	return SideEffectInstances;
}

void UNexusSideEffectComponent::UnregisterAndRemoveSideEffect(const FNexusSideEffectInstanceHandle& SideEffectInstanceHandle)
{
	SideEffectInstances.RemoveSideEffectInstance(SideEffectInstanceHandle);
}

FNexusSideEffectInstanceHandle UNexusSideEffectComponent::RegisterAndApplySideEffect(const FNexusSideEffectInstanceDef& SideEffectInstanceDef)
{
	return SideEffectInstances.ApplySideEffectByDef(SideEffectInstanceDef);
}
