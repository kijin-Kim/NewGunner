// Fill out your copyright notice in the Description page of Project Settings.


#include "Cue/NexusCue.h"

#include "NexusLog.h"


bool FNexusCueParameters::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	bOutSuccess = true;

	// TODO: 유효한지 체크하고 리플리케이트. 유효 플래그 추가
	Ar << Causer;
	Ar << Target;

	if (!Location.NetSerialize(Ar, Map, bOutSuccess))
	{
		return false;
	}

	if (!Normal.NetSerialize(Ar, Map, bOutSuccess))
	{
		return false;
	}

	uint8 HitResultCount = HitResults.Num();
	Ar << HitResultCount;
	if (Ar.IsLoading())
	{
		HitResults.SetNum(HitResultCount);
	}

	for (FHitResult& HitResult : HitResults)
	{
		if (!HitResult.NetSerialize(Ar, Map, bOutSuccess))
		{
			return false;
		}
	}

	return true;
}

void FNexusLoopingCueHandle::GenerateNewHandle()
{
	static int32 HandleCounter = 1;
	Handle = HandleCounter++;
}

FNexusLoopingCue::FNexusLoopingCue(): CueClass(nullptr)
{
	Handle.GenerateNewHandle();
}

bool FNexusLoopingCue::operator==(const FNexusLoopingCue& Other) const
{
	return Handle == Other.Handle && CueClass == Other.CueClass;
}

bool FNexusLoopingCue::operator!=(const FNexusLoopingCue& Other) const
{
	return !(*this == Other);
}

void FNexusLoopingCue::PreReplicatedRemove(const FNexusLoopingCueContainer& InArraySerializer)
{
	InArraySerializer.OnRemoved(*this);
}

FString FNexusLoopingCue::ToString() const
{
	return FString::Printf(TEXT("LoopingCue={Handle=%s, CueClass=%s}"), *Handle.ToString(), *CueClass->GetName());
}


void FNexusLoopingCueContainer::Init(TWeakObjectPtr<AActor> InAgentActor, TWeakObjectPtr<AActor> InOwnerActor)
{
	check(InOwnerActor.IsValid() && InAgentActor.IsValid());
	OwnerActor = InOwnerActor;
	AgentActor = InAgentActor;
	bInitialized = true;

	FlushPendingAdds();
}

bool FNexusLoopingCueContainer::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FNexusLoopingCue, FNexusLoopingCueContainer>(Items, DeltaParms, *this);
}

FNexusLoopingCueHandle FNexusLoopingCueContainer::AddLoopingCue(const FNexusLoopingCue& InLoopingCue, bool bHasAuthority)
{
	check(InLoopingCue.CueClass);

	int32 Index = Items.Add(InLoopingCue);
	if (bHasAuthority)
	{
		MarkItemDirty(Items[Index]);
	}
	OnAdded(Items[Index]);
	return Items[Index].GetHandle();
}

void FNexusLoopingCueContainer::RemoveLoopingCue(FNexusLoopingCueHandle Handle, bool bHasAuthority)
{
	FNexusLoopingCue* LoopingCue = FindLoopingCueByHandle(Handle);
	if (!LoopingCue)
	{
		return;
	}
	OnRemoved(*LoopingCue);
	int32 Removed = Items.RemoveAll([Handle](const FNexusLoopingCue& Other)
	{
		return Other.GetHandle() == Handle;
	});
	check(Removed != 0);
	MarkArrayDirty();
}

void FNexusLoopingCueContainer::RemoveAllLoopingCues()
{
	for (FNexusLoopingCue& LoopingCue : Items)
	{
		OnRemoved(LoopingCue);
	}
	Items.Empty();
	MarkArrayDirty();
}


void FNexusLoopingCueContainer::PostReplicatedReceive(const FFastArraySerializer::FPostReplicatedReceiveParameters& Parameters)
{
	// FNexusActionDefContainer::PostReplicatedReceive와 동일한 로직 수행. Causer, Target, HitResults 내부의 Actor 소환을 기다려야 함. 
	if (!Parameters.bHasMoreUnmappedReferences && bInitialized)
	{
		FlushPendingAdds();
		UE_LOG(LogNexusCue, VeryVerbose, TEXT("루핑큐 추가 플러시"));
	}
	else
	{
		UE_LOG(LogNexusCue, VeryVerbose, TEXT("매핑되지 않은 레퍼런스 존재"));
	}
}

FNexusLoopingCue* FNexusLoopingCueContainer::FindLoopingCueByHandle(const FNexusLoopingCueHandle& InHandle)
{
	return Items.FindByPredicate([InHandle](const FNexusLoopingCue& LoopingCue)
	{
		return LoopingCue.GetHandle() == InHandle;
	});
}

FNexusLoopingCue* FNexusLoopingCueContainer::FindLoopingCueByClass(TSubclassOf<ANexusCue> InCueClass)
{
	return Items.FindByPredicate([InCueClass](const FNexusLoopingCue& LoopingCue)
	{
		return LoopingCue.CueClass == InCueClass;
	});
}

void FNexusLoopingCueContainer::OnAdded(FNexusLoopingCue& LoopingCue) const
{
	LoopingCue.bIsAdded = true;
	if (OnCueAddedDelegate.IsBound())
	{
		OnCueAddedDelegate.Execute(LoopingCue);
	}
}

void FNexusLoopingCueContainer::OnRemoved(const FNexusLoopingCue& LoopingCue) const
{
	if (OnCueRemovedDelegate.IsBound())
	{
		OnCueRemovedDelegate.Execute(LoopingCue);
	}
}

void FNexusLoopingCueContainer::FlushPendingAdds()
{
	for (const auto& [OuterIndex, GuidRefeMap] : GuidReferencesMap_StructDelta)
	{
		for (const auto& [Index, GuidRefs] : GuidRefeMap)
		{
			if (GuidRefs.GetUnmappedGUIDs().Num() > 0)
			{
				return;
			}
		}
	}

	check(bInitialized);
	for (FNexusLoopingCue& Item : Items)
	{
		if (!Item.bIsAdded)
		{
			Item.bIsAdded = true;
			OnAdded(Item);
		}
	}
}

ANexusCue::ANexusCue()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ANexusCue::CallOnTriggered(const FNexusCueParameters& InCueParameters, AActor* AgentActor, AActor* OwnerActor)
{
	if (CueState == ENexusCueState::None || CueType == ENexusCueType::Burst)
	{
		CueState = ENexusCueState::Triggered;
		NX_LOG_SUB(AgentActor, LogNexusCue, Display, TEXT("큐 액터 실행: %s"), *GetName());
		OnTriggered(InCueParameters, AgentActor, OwnerActor);
		BP_OnTriggered(InCueParameters, AgentActor, OwnerActor);
	}
}

void ANexusCue::CallOnBecomeRelevant(const FNexusCueParameters& InCueParameters, AActor* AgentActor, AActor* OwnerActor)
{
	if (CueState != ENexusCueState::BecomeRelevant)
	{
		CueState = ENexusCueState::BecomeRelevant;
		NX_LOG_SUB(AgentActor, LogNexusCue, Display, TEXT("큐 액터 연관 시작: %s"), *GetName());
		OnBecomeRelevant(InCueParameters, AgentActor, OwnerActor);
		BP_OnBecomeRelevant(InCueParameters, AgentActor, OwnerActor);
	}
}

void ANexusCue::CallOnCeaseRelevant(AActor* AgentActor, AActor* OwnerActor)
{
	CueState = ENexusCueState::None;
	NX_LOG_SUB(AgentActor, LogNexusCue, Display, TEXT("큐 연관 종료: %s"), *GetName());
	OnCeaseRelevant(AgentActor, OwnerActor);
	BP_OnCeaseRelevant(AgentActor, OwnerActor);
}
