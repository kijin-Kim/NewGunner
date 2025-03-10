// Fill out your copyright notice in the Description page of Project Settings.


#include "Cue/NexusCue.h"

#include "NexusLog.h"


UWorld* UNexusCue::GetWorld() const
{
	// https://forums.unrealengine.com/t/can-you-use-a-blueprint-function-library-in-an-object-class/350918/37
	if (HasAllFlags(RF_ClassDefaultObject))
	{
		return nullptr;
	}
	return GetOuter()->GetWorld();
}

void UNexusCue::CallOnTriggered(const FNexusTargetDataHandle& InTargetDataHandle)
{
	TargetDataHandle = InTargetDataHandle;
	OnTriggered(InTargetDataHandle);
	BP_OnTriggered();
	TargetDataHandle = FNexusTargetDataHandle();
}

void UNexusCue::CallOnBecomeRelevant()
{
	OnBecomeRelevant();
	BP_OnBecomeRelevant();
}

void UNexusCue::CallOnCeaseRelevant()
{
	OnCeaseRelevant();
	BP_OnCeaseRelevant();
}

void UNexusCue::OnTriggered(const FNexusTargetDataHandle& InTargetDataHandle)
{
	UE_LOG(LogNexusCue, Log, TEXT( "[EditorID: %d] UNexusCue::OnTriggered" ), static_cast<int32>(GPlayInEditorID));
}

void UNexusCue::OnBecomeRelevant()
{
	UE_LOG(LogNexusCue, Log, TEXT( "[EditorID: %d] UNexusCue::OnBecomeRelevant" ), static_cast<int32>(GPlayInEditorID));
}

void UNexusCue::OnCeaseRelevant()
{
	UE_LOG(LogNexusCue, Log, TEXT( "[EditorID: %d] UNexusCue::OnCeaseRelevant" ), static_cast<int32>(GPlayInEditorID));
}
