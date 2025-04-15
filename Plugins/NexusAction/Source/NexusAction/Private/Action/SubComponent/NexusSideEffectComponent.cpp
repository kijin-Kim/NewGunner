// Fill out your copyright notice in the Description page of Project Settings.


#include "Action/SubComponent/NexusSideEffectComponent.h"

#include "NexusLog.h"
#include "Action/NexusAction.h"
#include "Net/UnrealNetwork.h"
#include "SideEffect/NexusSideEffect.h"


UNexusSideEffectComponent::UNexusSideEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UNexusSideEffectComponent::Setup(TSharedPtr<FNexusAgentInfo> InAgentInfo)
{
	Super::Setup(InAgentInfo);
	SideEffectDefs.Init(GetOwnerActor());
}

void UNexusSideEffectComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UNexusSideEffectComponent, SideEffectDefs, COND_OwnerOnly);
}

void UNexusSideEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SideEffectDefs.Tick(DeltaTime);
}

void UNexusSideEffectComponent::TriggerSideEffectByDef(const FNexusSideEffectDef& NewSideEffectDef, FNexusPredictionTag PredictionTag, FNexusPredictionEventSignature::FDelegate&& OnPredictionEnded, FNexusPredictionEventSignature::FDelegate&& OnPredictionFailed)
{
	if (!GetOwner()->HasAuthority() && !PredictionTag.IsPredictable())
	{
		NX_VLOG_SUB(GetOwner(), LogNexusSideEffect, Verbose, TEXT("예측 불가능한 예측 태그에서 사이드 이펙트를 실행할 수 없습니다"));
		return;
	}

	SideEffectDefs.Add(NewSideEffectDef, PredictionTag);
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

		PredictionEvent.OnPredictionEnded.AddWeakLambda(this, [this, SideEffectDefHandle = NewSideEffectDef.Handle, NewSideEffectDef]()
		{
			NX_VLOG_SUB(GetOwner(), LogNexusSideEffect, Log, TEXT("사이드 이펙트 [%s] 삭제 (예측 종료)"), *NewSideEffectDef.SideEffectClass->GetName());
			SideEffectDefs.Remove(SideEffectDefHandle);
		});

		PredictionEvent.OnPredictionFailed.AddWeakLambda(this, [this, SideEffectDefHandle = NewSideEffectDef.Handle, NewSideEffectDef]()
		{
			NX_VLOG_SUB(GetOwner(), LogNexusSideEffect, Error, TEXT("사이드 이펙트 [%s] 삭제 (예측 실패)"), *NewSideEffectDef.SideEffectClass->GetName());
			SideEffectDefs.Remove(SideEffectDefHandle);
		});
	}
}

void UNexusSideEffectComponent::RemoveSideEffect(FNexusSideEffectDefHandle SideEffectDefHandle)
{
	SideEffectDefs.Remove(SideEffectDefHandle);
}

const FNexusSideEffectDefContainer& UNexusSideEffectComponent::GetSideEffectDefs() const
{
	return SideEffectDefs;
}
