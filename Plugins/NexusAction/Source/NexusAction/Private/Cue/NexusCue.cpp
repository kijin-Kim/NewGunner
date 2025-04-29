// Fill out your copyright notice in the Description page of Project Settings.


#include "Cue/NexusCue.h"

#include "NexusLog.h"


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

void FNexusLoopingCue::PostReplicatedAdd(const FNexusLoopingCueContainer& InArraySerializer)
{
	InArraySerializer.OnAdded(*this);
}

void FNexusLoopingCue::PostReplicatedChange(const FNexusLoopingCueContainer& InArraySerializer)
{
	unimplemented();
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
	return Items[Index].Handle;
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
		return Other.Handle == Handle;
	});
	check(Removed != 0);
	if (bHasAuthority)
	{
		MarkArrayDirty();
	}
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

void FNexusLoopingCueContainer::OnAdded(FNexusLoopingCue& LoopingCue) const
{
	if (OnCueAddedDelegate.IsBound())
	{
		OnCueAddedDelegate.Execute(LoopingCue);
	}
}

void FNexusLoopingCueContainer::OnRemoved(FNexusLoopingCue& LoopingCue) const
{
	if (OnCueRemovedDelegate.IsBound())
	{
		OnCueRemovedDelegate.Execute(LoopingCue);
	}
}

FNexusLoopingCue* FNexusLoopingCueContainer::FindLoopingCueByHandle(const FNexusLoopingCueHandle& InHandle)
{
	return Items.FindByPredicate([InHandle](const FNexusLoopingCue& LoopingCue)
	{
		return LoopingCue.Handle == InHandle;
	});
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
	TargetDataHandle.Reset();
}

void ANexusCue::CallOnBecomeRelevant(const FNexusTargetDataHandle& InTargetDataHandle)
{
	TargetDataHandle = InTargetDataHandle;
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
	TargetDataHandle.Reset();
}

void ANexusCue::EndCue() const
{
	check(HasAuthority());
	OnDurationExpiredDelegate.ExecuteIfBound();
}

void ANexusCue::OnTriggered(const FNexusTargetDataHandle& InTargetDataHandle)
{
}

void ANexusCue::OnBecomeRelevant()
{
}

void ANexusCue::OnCeaseRelevant()
{
}
