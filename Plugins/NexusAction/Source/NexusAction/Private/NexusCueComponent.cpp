// Fill out your copyright notice in the Description page of Project Settings.


#include "NexusCueComponent.h"

#include "Action/NexusAgentInfo.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UNexusCueComponent::UNexusCueComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UNexusCueComponent::Init(TSharedPtr<FNexusAgentInfo> InAgentInfo)
{
	AgentInfo = InAgentInfo;

	LoopingCues.OnCueAddedDelegate.BindUObject(this, &UNexusCueComponent::OnCueAdded);
	LoopingCues.OnCueRemovedDelegate.BindUObject(this, &UNexusCueComponent::OnCueRemoved);

	for (FNexusLoopingCue& LoopingCue : LoopingCues.Items)
	{
		LoopingCues.OnAdded(LoopingCue);
	}
}

void UNexusCueComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UNexusCueComponent, LoopingCues);
}

void UNexusCueComponent::TriggerCue(const FNexusTriggerCueParams& TriggerCueParams)
{
	ANexusCue* CueCDO = TriggerCueParams.CueClass.GetDefaultObject();
	if (CueCDO->GetCueType() != ENexusCueType::Looping)
	{
		return;
	}

	if (!TriggerCueParams.CueClass)
	{
		NX_LOG_SUB(LogNexusAction, Verbose, TEXT("CueClass가 유효하지 않습니다"));
		return;
	}


	FNexusLoopingCueHandle Handle;
	if (CueCDO->GetCueType() == ENexusCueType::Looping)
	{
		FNexusLoopingCue NewLoopingCue;
		NewLoopingCue.CueClass = TriggerCueParams.CueClass;
		NewLoopingCue.TargetDataHandle = TriggerCueParams.TargetDataHandle;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = AgentInfo->GetAgentActor();
		NewLoopingCue.CueActor = GetWorld()->SpawnActor<ANexusCue>(NewLoopingCue.CueClass, AgentInfo->GetAgentActor()->GetActorTransform(), SpawnParams);
		NewLoopingCue.CueActor->CallOnTriggered(TriggerCueParams.TargetDataHandle);
		Handle = LoopingCues.AddLoopingCue(NewLoopingCue, GetOwner()->HasAuthority());
	}

	if (!GetOwner()->HasAuthority())
	{
		if (TriggerCueParams.PredictionTag.IsPredictable())
		{
			if (CueCDO->GetCueType() == ENexusCueType::Looping)
			{
				if (!GetOwner()->HasAuthority() && TriggerCueParams.PredictionTag.IsPredictable())
				{
					FNexusPredictionEvents::FPredictionEvent& PredictionEvent = FNexusPredictionEvents::GetPredictionEvent(TriggerCueParams.PredictionTag);
					PredictionEvent.OnPredictionEnded.AddWeakLambda(this, [this, Handle]()
					{
						LoopingCues.RemoveLoopingCue(Handle, GetOwner()->HasAuthority());
					});

					PredictionEvent.OnPredictionFailed.AddWeakLambda(this, [this, Handle]()
					{
						LoopingCues.RemoveLoopingCue(Handle, GetOwner()->HasAuthority());
					});
				}
			}
		}
	}
	else if (GetCueNetworkProxyInterface())
	{
		GetCueNetworkProxyInterface()->CallNetMulticastTriggerCue(TriggerCueParams, Handle);
	}
}


void UNexusCueComponent::AuthEndCue(FNexusLoopingCueHandle CueHandle)
{
	if (!GetOwner()->HasAuthority())
	{
		NX_LOG_SUB(LogNexusAction, Warning, TEXT("함수는 서버에서만 호출 가능합니다."));
		return;
	}

	FNexusLoopingCue* LoopingCue = LoopingCues.FindLoopingCueByHandle(CueHandle);
	if (LoopingCue)
	{
		LoopingCue->CueActor->EndCue();
	}
}



void UNexusCueComponent::SimTriggerCue(const FNexusTriggerCueParams& TriggerCueParams, FNexusLoopingCueHandle ServerCueHandle)
{
	if (!GetOwner()->HasAuthority() && !TriggerCueParams.PredictionTag.IsPredictable())
	{
		FNexusLoopingCue* LoopingCue = LoopingCues.FindLoopingCueByHandle(ServerCueHandle);
		if (!LoopingCue)
		{
			FNexusLoopingCue NewLoopingCue;
			NewLoopingCue.CueClass = TriggerCueParams.CueClass;
			NewLoopingCue.TargetDataHandle = TriggerCueParams.TargetDataHandle;
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = AgentInfo->GetAgentActor();
			NewLoopingCue.CueActor = GetWorld()->SpawnActor<ANexusCue>(NewLoopingCue.CueClass, AgentInfo->GetAgentActor()->GetActorTransform(), SpawnParams);
			NewLoopingCue.CueActor->CallOnTriggered(TriggerCueParams.TargetDataHandle);
			NewLoopingCue.Handle = ServerCueHandle;
			LoopingCues.AddLoopingCue(NewLoopingCue, true);
		}
	}
}

void UNexusCueComponent::RemoveAllLoopingCues()
{
	 LoopingCues.RemoveAllLoopingCues();
}

void UNexusCueComponent::NetMulticastTriggerCue_Implementation(const FNexusTriggerCueParams& TriggerCueParams, FNexusLoopingCueHandle ServerCueHandle)
{
	SimTriggerCue(TriggerCueParams, ServerCueHandle);
}


INexusCueNetworkProxyInterface* UNexusCueComponent::GetCueNetworkProxyInterface()
{
	if (GetOwner()->bAlwaysRelevant)
	{
		return Cast<INexusCueNetworkProxyInterface>(AgentInfo->AgentActor);
	}

	return this;
}

void UNexusCueComponent::OnCueAdded(FNexusLoopingCue& NexusLoopingCue)
{
	
	if (!AgentInfo->GetAgentActor())
	{
		NX_LOG_SUB_FN(LogNexusCue, Verbose, TEXT("AgentActor가 유효하지 않습니다"));
		return;
	}

	if (!NexusLoopingCue.CueActor)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = AgentInfo->GetAgentActor();
		NexusLoopingCue.CueActor = GetWorld()->SpawnActor<ANexusCue>(NexusLoopingCue.CueClass, AgentInfo->GetAgentActor()->GetActorTransform(), SpawnParams);
		check(NexusLoopingCue.CueActor);
	}

	NexusLoopingCue.CueActor->CallOnBecomeRelevant(NexusLoopingCue.TargetDataHandle);
	if (GetOwner()->HasAuthority())
	{
		NexusLoopingCue.CueActor->OnDurationExpiredDelegate.BindWeakLambda(this, [this, NexusLoopingCue]()
		{
			if (NexusLoopingCue.CueClass)
			{
				LoopingCues.RemoveLoopingCue(NexusLoopingCue.Handle, GetOwner()->HasAuthority());
			}
		});
	}
}


void UNexusCueComponent::OnCueRemoved(FNexusLoopingCue& NexusLoopingCue)
{
	if (NexusLoopingCue.CueActor)
	{
		NexusLoopingCue.CueActor->CallOnCeaseRelevant();
		NexusLoopingCue.CueActor->Destroy();
	}
}
