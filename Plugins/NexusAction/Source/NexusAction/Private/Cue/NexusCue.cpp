// Fill out your copyright notice in the Description page of Project Settings.


#include "Cue/NexusCue.h"

#include "NexusLog.h"


void FNexusLoopingCue::PreReplicatedRemove(const FNexusLoopingCueContainer& InArraySerializer)
{
	UE_LOG(LogNexusCue, Log, TEXT("FNexusLoopingCue::PreReplicatedRemove [%s]"), *GetNameSafe(CueClass));
	InArraySerializer.OnRemoved(CueClass);
}

void FNexusLoopingCue::PostReplicatedAdd(const FNexusLoopingCueContainer& InArraySerializer)
{
	UE_LOG(LogNexusCue, Log, TEXT("FNexusLoopingCue::PostReplicatedAdd [%s]"), *GetNameSafe(CueClass));
	InArraySerializer.OnAdded(*this);
}

void FNexusLoopingCue::PostReplicatedChange(const FNexusLoopingCueContainer& InArraySerializer)
{
	checkNoEntry();
	UE_LOG(LogNexusCue, Log, TEXT("FNexusLoopingCue::PostReplicatedChange [%s]"), *GetNameSafe(CueClass));
}

bool FNexusLoopingCueContainer::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FNexusLoopingCue, FNexusLoopingCueContainer>(Items, DeltaParms, *this);
}

void FNexusLoopingCueContainer::AddLoopingCue(const FNexusLoopingCue& InLoopingCue, bool bHasAuthority)
{
	check(InLoopingCue.CueClass);
	OnAdded(InLoopingCue);
	int32 Index = Items.Add(InLoopingCue);
	if (bHasAuthority)
	{
		MarkItemDirty(Items[Index]);
	}
	// else
	// {
	// 	MarkArrayDirty();
	// }
}

void FNexusLoopingCueContainer::RemoveLoopingCue(TSubclassOf<ANexusCue> InCueClass, bool bHasAuthority)
{
	Items.RemoveAll([InCueClass](const FNexusLoopingCue& LoopingCue)
	{
		return LoopingCue.CueClass == InCueClass;
	});

	OnRemoved(InCueClass);
	if (bHasAuthority)
	{
		MarkArrayDirty();
	}
}

void FNexusLoopingCueContainer::RemoveAllLoopingCues()
{
	for (const FNexusLoopingCue& LoopingCue : Items)
	{
		OnRemoved(LoopingCue.CueClass);
	}
	Items.Empty();
	MarkArrayDirty();
}

void FNexusLoopingCueContainer::OnAdded(const FNexusLoopingCue& LoopingCue) const
{
	if (OnCueAddedDelegate.IsBound())
	{
		OnCueAddedDelegate.Execute(LoopingCue);
	}
}

void FNexusLoopingCueContainer::OnRemoved(TSubclassOf<ANexusCue> CueClass) const
{
	if (OnCueRemovedDelegate.IsBound())
	{
		OnCueRemovedDelegate.Execute(CueClass);
	}
}

ANexusCue::ANexusCue()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ANexusCue::CallOnTriggered(const FNexusTargetDataHandle& InTargetDataHandle)
{
	TargetDataHandle = InTargetDataHandle;
	OnTriggered(InTargetDataHandle);
	BP_OnTriggered();
	TargetDataHandle = FNexusTargetDataHandle();
}

void ANexusCue::CallOnBecomeRelevant()
{
	OnBecomeRelevant();
	BP_OnBecomeRelevant();

	if (HasAuthority() && CueType == ENexusCueType::Looping && Duration > 0.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(DurationExpiredTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(DurationExpiredTimerHandle, this, &ANexusCue::EndCue, Duration, false);
	}
}

void ANexusCue::CallOnCeaseRelevant()
{
	OnCeaseRelevant();
	BP_OnCeaseRelevant();
}

void ANexusCue::OnTriggered(const FNexusTargetDataHandle& InTargetDataHandle)
{
	UE_LOG(LogNexusCue, Log, TEXT( "[EditorID: %d] 큐 [%s] 트리거"), static_cast<int32>(GPlayInEditorID), *GetNameSafe(this));
}

void ANexusCue::OnBecomeRelevant()
{
	UE_LOG(LogNexusCue, Log, TEXT( "[EditorID: %d] 큐 [%s] 활성화"), static_cast<int32>(GPlayInEditorID), *GetNameSafe(this));
}

void ANexusCue::OnCeaseRelevant()
{
	UE_LOG(LogNexusCue, Log, TEXT( "[EditorID: %d] 큐 [%s] 비활성화"), static_cast<int32>(GPlayInEditorID), *GetNameSafe(this));
}

void ANexusCue::EndCue() const
{
	check(HasAuthority());
	OnDurationExpiredDelegate.ExecuteIfBound();
}
