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
		NX_VLOG_SUB(GetOwner(), LogNexusSideEffect, Verbose, TEXT("예측 불가능한 예측 태그에서 사이드 이펙트를 실행할 수 없습니다"));
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

		PredictionEvent.OnPredictionEnded.AddWeakLambda(this, [this,SideEffectInstanceHandle, SideEffectName = SideEffectInstanceDef.SideEffectAsset->GetName()]()
		{
			NX_VLOG_SUB(GetOwner(), LogNexusSideEffect, Log, TEXT("사이드 이펙트 [%s] 삭제 (예측 종료)"), *SideEffectName);
			UnregisterAndRemoveSideEffect(SideEffectInstanceHandle);
		});

		PredictionEvent.OnPredictionFailed.AddWeakLambda(this, [this,SideEffectInstanceHandle, SideEffectName = SideEffectInstanceDef.SideEffectAsset->GetName()]()
		{
			NX_VLOG_SUB(GetOwner(), LogNexusSideEffect, Error, TEXT("사이드 이펙트 [%s] 삭제 (예측 실패)"), *SideEffectName);
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
