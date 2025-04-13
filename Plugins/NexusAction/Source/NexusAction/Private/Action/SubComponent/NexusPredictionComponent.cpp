// Fill out your copyright notice in the Description page of Project Settings.


#include "Action/SubComponent/NexusPredictionComponent.h"

#include "Prediction/NexusPredictionScope.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UNexusPredictionComponent::UNexusPredictionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UNexusPredictionComponent::Setup(TSharedPtr<FNexusAgentInfo> InAgentInfo)
{
	Super::Setup(InAgentInfo);
	NetPredictionTags.Init(GetOwner()->HasAuthority());
}

void UNexusPredictionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UNexusPredictionComponent, NetPredictionTags, COND_OwnerOnly);
}

void UNexusPredictionComponent::ReplicateNetPredictionTag(const FNexusPredictionTag& PredictionTag)
{
	check(PredictionTag.IsValid());
	NetPredictionTags.ReplicateNetPredictionTag(PredictionTag);
}

void UNexusPredictionComponent::SetCurrentPredictionTag(const FNexusPredictionTag& NewTag)
{
	CurrentPredictionTag = NewTag;
}

FNexusPredictionTag UNexusPredictionComponent::GetCurrentPredictionTag() const
{
	return CurrentPredictionTag;
}

void UNexusPredictionComponent::ServerSendNetSyncPoint_Implementation(FNexusActionDefHandle Handle, FNexusPredictionTag PrimaryPredictionTag, FNexusPredictionTag PredictionTag)
{
	const FNexusRepDataKey Key{Handle, PrimaryPredictionTag};
	FNexusNetSyncDelegate* RepDataDelegate = NetSyncPointDelegates.Find(Key);
	if (!RepDataDelegate)
	{
		NetSyncPointDelegates.Add(Key, PredictionTag);
		return;
	}

	FNexusNetSyncDelegate CopiedDelegate = *RepDataDelegate;
	NetSyncPointDelegates.Remove(Key);
	if (CopiedDelegate.OnSyncDelegate.IsBound())
	{
		FNexusPredictionScope PredictionScope(*this, PredictionTag);
		CopiedDelegate.OnSyncDelegate.Broadcast();
	}
}


void UNexusPredictionComponent::CallOrAddNetsyncPointDelegate(FNexusActionDefHandle Handle, FNexusPredictionTag PrimaryPredictionTag, FSimpleMulticastDelegate::FDelegate&& Delegate)
{
	const FNexusRepDataKey Key{Handle, PrimaryPredictionTag};
	FNexusNetSyncDelegate* RepDataDelegate = NetSyncPointDelegates.Find(Key);
	if (!RepDataDelegate)
	{
		NetSyncPointDelegates.Add(Key, FNexusNetSyncDelegate{PrimaryPredictionTag, MoveTemp(Delegate)});
		return;
	}

	FNexusPredictionScope PredictionScope(*this, RepDataDelegate->PredictionTag);
	NetSyncPointDelegates.Remove(Key);
	Delegate.ExecuteIfBound();
}

void UNexusPredictionComponent::ServerSendTargetData_Implementation(FNexusActionDefHandle Handle, FNexusPredictionTag PrimaryPredictionTag, FNexusPredictionTag PredictionTag, FNexusTargetDataHandle TargetDataHandle)
{
	const FNexusRepDataKey Key{Handle, PrimaryPredictionTag};
	FNexusTargetDataDelegate* RepDataDelegate = TargetDataDelegates.Find(Key);
	if (!RepDataDelegate)
	{
		TargetDataDelegates.Add(Key, {PredictionTag, TargetDataHandle});
		return;
	}

	FNexusTargetDataDelegate CopiedDelegate = *RepDataDelegate;
	TargetDataDelegates.Remove(Key);
	if (CopiedDelegate.OnSetDelegate.IsBound())
	{
		FNexusPredictionScope PredictionScope(*this, PredictionTag);
		CopiedDelegate.OnSetDelegate.Broadcast(TargetDataHandle);
	}
}

void UNexusPredictionComponent::CallOrAddTargetDataDelegate(FNexusActionDefHandle Handle, FNexusPredictionTag PrimaryPredictionTag, FOnNexusTargetDataSetSignature::FDelegate&& Delegate)
{
	const FNexusRepDataKey Key{Handle, PrimaryPredictionTag};
	FNexusTargetDataDelegate* RepDataDelegate = TargetDataDelegates.Find(Key);
	if (!RepDataDelegate)
	{
		TargetDataDelegates.Add(Key, MoveTemp(Delegate));
		return;
	}

	FNexusPredictionScope PredictionScope(*this, RepDataDelegate->PredictionTag);
	FNexusTargetDataHandle CopiedHandle = RepDataDelegate->TargetDataHandle;
	TargetDataDelegates.Remove(Key);
	Delegate.ExecuteIfBound(CopiedHandle);
}

