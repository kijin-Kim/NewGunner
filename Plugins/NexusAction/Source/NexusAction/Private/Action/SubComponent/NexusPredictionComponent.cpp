// Fill out your copyright notice in the Description page of Project Settings.


#include "Action/SubComponent/NexusPredictionComponent.h"

#include "Prediction/NexusPredictionScope.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UNexusPredictionComponent::UNexusPredictionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
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


	if (!GetOwner()->HasAuthority() && CurrentPredictionTag.IsPredictable())
	{
		FNexusPredictionEvents::FPredictionEvent& PredictionEvent = FNexusPredictionEvents::GetPredictionEvent(CurrentPredictionTag);
		PredictionEvent.OnPredictionEnded.AddWeakLambda(this, [this, PredictionTagString = CurrentPredictionTag.ToString()]()
		{
			NX_VLOG_SUB(GetAgentActor(), LogNexusPrediction, Verbose, TEXT("예측태그 도착 (성공): %s"), *PredictionTagString);
		});

		PredictionEvent.OnPredictionFailed.AddWeakLambda(this, [this, PredictionTagString = CurrentPredictionTag.ToString()]()
		{
			NX_VLOG_SUB(GetAgentActor(), LogNexusPrediction, Verbose, TEXT("예측태그 도착 (실패): %s"), *PredictionTagString);
		});
	}
}

FNexusPredictionTag UNexusPredictionComponent::GetCurrentPredictionTag() const
{
	return CurrentPredictionTag;
}

void UNexusPredictionComponent::ServerSendNetSyncPoint_Implementation(const FNexusActionDefHandle& Handle, FNexusPredictionTag PrimaryPredictionTag, FNexusPredictionTag PredictionTag)
{
	const FNexusRepDataKey Key{Handle, PrimaryPredictionTag};
	FNexusNetSyncDelegate& RepDataDelegate = NetSyncPointDelegates.FindOrAdd(Key);
	NX_LOG_SUB(GetAgentActor(), LogNexusPrediction, Verbose, TEXT("서버 넷싱크포인트 수신: Key=%s, Current%s"), *Key.ToString(), *PredictionTag.ToString());

	RepDataDelegate.PredictionTag = PredictionTag;
	if (RepDataDelegate.OnSyncDelegate.IsBound())
	{
		FNexusPredictionScope PredictionScope(*this, RepDataDelegate.PredictionTag, TEXT("NetSyncPoint"));
		FNexusNetSyncDelegate CopiedDelegate = RepDataDelegate;
		RepDataDelegate.Reset();
		CopiedDelegate.OnSyncDelegate.Broadcast();
	}
}


void UNexusPredictionComponent::AuthCallOrAddNetsyncPointDelegate(const FNexusActionDefHandle& Handle, FNexusPredictionTag PrimaryPredictionTag, FSimpleMulticastDelegate::FDelegate&& Delegate)
{
	if (!IsOwnerActorAuthoritative())
	{
		NX_LOG_SUB(GetAgentActor(), LogNexusPrediction, Error, TEXT("권한 없는 함수 호출"));
		return;
	}

	const FNexusRepDataKey Key{Handle, PrimaryPredictionTag};

	FNexusNetSyncDelegate& RepDataDelegate = NetSyncPointDelegates.FindOrAdd(Key);
	RepDataDelegate.OnSyncDelegate.Add(MoveTemp(Delegate));
	if (RepDataDelegate.PredictionTag.IsValid())
	{
		FNexusPredictionScope PredictionScope(*this, RepDataDelegate.PredictionTag, TEXT("NetSyncPoint"));
		FNexusNetSyncDelegate CopiedDelegate = RepDataDelegate;
		RepDataDelegate.Reset();
		CopiedDelegate.OnSyncDelegate.Broadcast();
	}
}

void UNexusPredictionComponent::ServerSendTargetData_Implementation(const FNexusActionDefHandle& Handle, FNexusPredictionTag PrimaryPredictionTag, FNexusPredictionTag PredictionTag, const FNexusTargetDataHandle& TargetDataHandle)
{
	check(TargetDataHandle.IsValid());
	check(PrimaryPredictionTag.IsValid() && PredictionTag.IsValid());
	const FNexusRepDataKey Key{Handle, PrimaryPredictionTag};
	FNexusTargetDataDelegate& RepDataDelegate = TargetDataDelegates.FindOrAdd(Key);
	if (RepDataDelegate.PredictionTag.IsValid())
	{
		NX_LOG_SUB(GetAgentActor(), LogNexusPrediction, Verbose, TEXT("서버 타깃데이터 수신 (오버라이드): Key=%s, Current%s"), *Key.ToString(), *PredictionTag.ToString());
	}
	else
	{
		NX_LOG_SUB(GetAgentActor(), LogNexusPrediction, Verbose, TEXT("서버 타깃데이터 수신: Key=%s, Current%s"), *Key.ToString(), *PredictionTag.ToString());
	}

	RepDataDelegate.TargetDataHandle = TargetDataHandle;
	RepDataDelegate.PredictionTag = PredictionTag;

	if (RepDataDelegate.OnSetDelegate.IsBound())
	{
		FNexusPredictionScope PredictionScope(*this, RepDataDelegate.PredictionTag, TEXT("TargetData"));
		FNexusTargetDataDelegate CopiedDelegate = RepDataDelegate;
		RepDataDelegate.Reset();
		CopiedDelegate.OnSetDelegate.Broadcast(CopiedDelegate.TargetDataHandle);
	}
}

void UNexusPredictionComponent::AuthCallOrAddTargetDataDelegate(const FNexusActionDefHandle& Handle, FNexusPredictionTag PrimaryPredictionTag, FOnNexusTargetDataSetSignature::FDelegate&& Delegate)
{
	if (!IsOwnerActorAuthoritative())
	{
		NX_LOG_SUB(GetAgentActor(), LogNexusPrediction, Error, TEXT("권한 없는 함수 호출"));
		return;
	}


	const FNexusRepDataKey Key{Handle, PrimaryPredictionTag};
	FNexusTargetDataDelegate& RepDataDelegate = TargetDataDelegates.FindOrAdd(Key);
	if (RepDataDelegate.OnSetDelegate.IsBound())
	{
		NX_LOG_SUB(GetAgentActor(), LogNexusPrediction, Verbose, TEXT("타깃데이터 델리게이트 추가 (오버라이드): Key=%s"), *Key.ToString());
	}
	else
	{
		NX_LOG_SUB(GetAgentActor(), LogNexusPrediction, Verbose, TEXT("타깃데이터 델리게이트 추가: Key=%s"), *Key.ToString());
	}

	RepDataDelegate.OnSetDelegate.Add(MoveTemp(Delegate));
	if (RepDataDelegate.PredictionTag.IsValid())
	{
		FNexusPredictionScope PredictionScope(*this, RepDataDelegate.PredictionTag, TEXT("TargetData"));
		FNexusTargetDataDelegate CopiedDelegate = RepDataDelegate;
		RepDataDelegate.Reset();
		CopiedDelegate.OnSetDelegate.Broadcast(CopiedDelegate.TargetDataHandle);
	}
}

void UNexusPredictionComponent::AuthClearAllReplicationDelegates(const FNexusActionDefHandle& Handle, FNexusPredictionTag PrimaryPredictionTag)
{
	if (!IsOwnerActorAuthoritative())
	{
		NX_LOG_SUB(GetAgentActor(), LogNexusPrediction, Error, TEXT("권한 없는 함수 호출"));
		return;
	}

	const FNexusRepDataKey Key{Handle, PrimaryPredictionTag};
	NX_LOG_SUB(GetAgentActor(), LogNexusPrediction, Verbose, TEXT("모든 리플리케이션 델리게이트 제거: Key=%s"), *Key.ToString());
	NetSyncPointDelegates.Remove(Key);
	TargetDataDelegates.Remove(Key);
}
