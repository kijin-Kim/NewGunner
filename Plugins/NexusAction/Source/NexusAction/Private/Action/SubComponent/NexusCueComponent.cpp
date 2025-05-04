// Fill out your copyright notice in the Description page of Project Settings.


#include "Action/SubComponent/NexusCueComponent.h"

#include "NexusLog.h"
#include "Action/NexusAgentInfo.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UNexusCueComponent::UNexusCueComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UNexusCueComponent::Setup(TSharedPtr<FNexusAgentInfo> InAgentInfo)
{
	Super::Setup(InAgentInfo);

	LoopingCues.OnCueAddedDelegate.BindUObject(this, &UNexusCueComponent::OnCueAdded);
	LoopingCues.OnCueRemovedDelegate.BindUObject(this, &UNexusCueComponent::OnCueRemoved);
	LoopingCues.Init(AgentInfo->GetAgentActor(), AgentInfo->GetOwnerActor());
}

void UNexusCueComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UNexusCueComponent, LoopingCues);
}

void UNexusCueComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (auto It = LocalLoopingCueActors.CreateIterator(); It; ++It)
	{
		if (!It->Value)
		{
			It.RemoveCurrent();
			continue;
		}

		if (It->Value->GetCueState() != ENexusCueState::BecomeRelevant)
		{
			continue;
		}

		if (LocalLoopingCueActorsCount.FindOrAdd(It->Key) <= 0)
		{
			It->Value->CallOnCeaseRelevant(AgentInfo->GetAgentActor(), AgentInfo->GetOwnerActor());
			It->Value->Destroy();
			It.RemoveCurrent();
		}
	}
}

void UNexusCueComponent::TriggerCue(TSubclassOf<ANexusCue> CueClass, FNexusPredictionTag PredictionTag, const FNexusCueParameters& CueParameters)
{
	if (!CueClass)
	{
		NX_LOG_SUB(GetAgentActor(), LogNexusCue, Error, TEXT("유효하지 않은 큐 클래스"));
		return;
	}


	ANexusCue* CueCDO = CueClass.GetDefaultObject();
	if (CueCDO->GetCueType() == ENexusCueType::Burst) // TODO : Looping이 아닌 경우도 처리
	{
		CueCDO->CallOnTriggered(CueParameters, AgentInfo->GetAgentActor(), AgentInfo->GetOwnerActor());
		if (IsOwnerActorAuthoritative() && GetCueNetworkProxyInterface())
		{
			GetCueNetworkProxyInterface()->CallNetMulticastTriggerCue(CueClass, PredictionTag, CueParameters);
		}
		return;
	}

	ANexusCue* LoopingCueActor = FindOrCreateLoopingCueActor(CueClass);
	LoopingCueActor->CallOnTriggered(CueParameters, AgentInfo->GetAgentActor(), AgentInfo->GetOwnerActor());
	FNexusLoopingCue NewLoopingCue{CueClass, CueParameters};
	FNexusLoopingCueHandle LoopingCueHandle = LoopingCues.AddLoopingCue(NewLoopingCue, IsOwnerActorAuthoritative());

	if (!IsOwnerActorAuthoritative())
	{
		if (PredictionTag.IsPredictable())
		{
			if (CueCDO->GetCueType() == ENexusCueType::Looping)
			{
				if (!IsOwnerActorAuthoritative() && PredictionTag.IsPredictable())
				{
					FNexusPredictionEvents::FPredictionEvent& PredictionEvent = FNexusPredictionEvents::GetPredictionEvent(PredictionTag);
					PredictionEvent.OnPredictionEnded.AddWeakLambda(this, [this, LoopingCueHandle]()
					{
						LoopingCues.RemoveLoopingCue(LoopingCueHandle, IsOwnerActorAuthoritative());
					});

					PredictionEvent.OnPredictionFailed.AddWeakLambda(this, [this, LoopingCueHandle]()
					{
						LoopingCues.RemoveLoopingCue(LoopingCueHandle, IsOwnerActorAuthoritative());
					});
				}
			}
		}
	}
	else if (GetCueNetworkProxyInterface())
	{
		GetCueNetworkProxyInterface()->CallNetMulticastTriggerCue(CueClass, PredictionTag, CueParameters);
	}
}

void UNexusCueComponent::AuthEndCue(TSubclassOf<ANexusCue> CueClass)
{
	if (!GetOwner()->HasAuthority())
	{
		NX_LOG_SUB(GetAgentActor(), LogNexusCue, Error, TEXT("권한 없는 함수 호출"));
		return;
	}

	if (FNexusLoopingCue* LoopingCue = LoopingCues.FindLoopingCueByClass(CueClass))
	{
		InternalAuthEndCue(LoopingCue->GetHandle());
	}
}

void UNexusCueComponent::InternalAuthEndCue(FNexusLoopingCueHandle LoopingCueHandle)
{
	if (!GetOwner()->HasAuthority())
	{
		NX_LOG_SUB(GetAgentActor(), LogNexusCue, Error, TEXT("권한 없는 함수 호출"));
		return;
	}

	LoopingCues.RemoveLoopingCue(LoopingCueHandle, GetOwner()->HasAuthority());
}

void UNexusCueComponent::SimTriggerCue(TSubclassOf<ANexusCue> CueClass, FNexusPredictionTag PredictionTag, const FNexusCueParameters& CueParameters)
{
	if (!GetOwner()->HasAuthority() && !PredictionTag.IsPredictable() && CueClass)
	{
		ANexusCue* CueCDO = CueClass->GetDefaultObject<ANexusCue>();
		ANexusCue* LoopingCueActor = CueCDO->GetCueType() == ENexusCueType::Burst ? CueCDO : FindOrCreateLoopingCueActor(CueClass);
		LoopingCueActor->CallOnTriggered(CueParameters, AgentInfo->GetAgentActor(), AgentInfo->GetOwnerActor());
	}
}

void UNexusCueComponent::RemoveAllLoopingCues()
{
	LoopingCues.RemoveAllLoopingCues();
	LocalLoopingCueActorsCount.Empty();
	for (auto& LoopingCueActor : LocalLoopingCueActors)
	{
		if (LoopingCueActor.Value)
		{
			LoopingCueActor.Value->CallOnCeaseRelevant(AgentInfo->GetAgentActor(), AgentInfo->GetOwnerActor());
			LoopingCueActor.Value->Destroy();
		}
	}
	LocalLoopingCueActors.Empty();
}

void UNexusCueComponent::NetMulticastTriggerCue_Implementation(TSubclassOf<ANexusCue> CueClass, FNexusPredictionTag PredictionTag, const FNexusCueParameters& CueParameters)
{
	SimTriggerCue(CueClass, PredictionTag, CueParameters);
}


INexusCueNetworkProxyInterface* UNexusCueComponent::GetCueNetworkProxyInterface()
{
	if (GetOwner()->bAlwaysRelevant)
	{
		return Cast<INexusCueNetworkProxyInterface>(AgentInfo->AgentActor);
	}

	return this;
}

void UNexusCueComponent::OnCueAdded(const FNexusLoopingCue& LoopingCue)
{
	if (!LoopingCue.CueClass)
	{
		NX_LOG_SUB(GetAgentActor(), LogNexusCue, Error, TEXT("유효하지 않은 큐 클래스"));
		return;
	}
	NX_LOG_SUB(GetAgentActor(), LogNexusCue, Log, TEXT("루핑큐 카운트 증가: %s"), *LoopingCue.ToString());

	// 1. SimTriggerCue -> AddLoopingCue -> OnCueAdded
	// 2. PostReplicatedAdd -> OnCueAdded
	ANexusCue* LoopingCueActor = FindOrCreateLoopingCueActor(LoopingCue.CueClass);
	LocalLoopingCueActorsCount.FindOrAdd(LoopingCue.CueClass)++;
	if (IsOwnerActorAuthoritative()
		&& LoopingCueActor->GetCueState() != ENexusCueState::BecomeRelevant
		&& LoopingCueActor->GetCueType() == ENexusCueType::Looping && LoopingCueActor->GetDuration() > 0.0f)
	{
		FTimerHandle ExpireTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(ExpireTimerHandle, [this, Handle = LoopingCue.GetHandle()]()
		{
			LoopingCues.RemoveLoopingCue(Handle, GetOwner()->HasAuthority());
		}, LoopingCueActor->GetDuration(), false);
	}
	LoopingCueActor->CallOnBecomeRelevant(LoopingCue.CueParameters, AgentInfo->GetAgentActor(), AgentInfo->GetOwnerActor());
}

void UNexusCueComponent::OnCueRemoved(const FNexusLoopingCue& LoopingCue)
{
	NX_LOG_SUB(GetAgentActor(), LogNexusCue, Log, TEXT("루핑큐 카운트 감소: %s"), *LoopingCue.ToString());
	LocalLoopingCueActorsCount.FindOrAdd(LoopingCue.CueClass)--;
}

ANexusCue* UNexusCueComponent::FindOrCreateLoopingCueActor(TSubclassOf<ANexusCue> CueClass)
{
	TObjectPtr<ANexusCue>& LoopingCueActor = LocalLoopingCueActors.FindOrAdd(CueClass);

	if (!LoopingCueActor)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = AgentInfo->GetAgentActor();
		LoopingCueActor = GetWorld()->SpawnActor<ANexusCue>(CueClass, AgentInfo->GetAgentActor()->GetActorTransform(), SpawnParams);
		NX_LOG_SUB(GetAgentActor(), LogNexusCue, Verbose, TEXT("루핑큐 액터 생성: %s"), *LoopingCueActor->GetName());
	}

	return LoopingCueActor;
}
