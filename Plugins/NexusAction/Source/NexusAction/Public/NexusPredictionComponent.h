// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NexusDataReplication.h"
#include "NexusPrediction.h"
#include "Components/ActorComponent.h"
#include "TargetData/NexusTargetData.h"
#include "NexusPredictionComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NEXUSACTION_API UNexusPredictionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNexusPredictionComponent();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void Init();
	
	void ReplicateNetPredictionTag(const FNexusPredictionTag& PredictionTag);
	void SetCurrentPredictionTag(const FNexusPredictionTag& NewTag);
	FNexusPredictionTag GetCurrentPredictionTag() const;

	UFUNCTION(Server, Reliable)
	void ServerSendNetSyncPoint(FNexusActionDefHandle Handle, FNexusPredictionTag PrimaryPredictionTag, FNexusPredictionTag PredictionTag);
	void CallOrAddNetsyncPointDelegate(FNexusActionDefHandle Handle, FNexusPredictionTag PrimaryPredictionTag, FSimpleMulticastDelegate::FDelegate&& Delegate);

	UFUNCTION(Server, Reliable)
	void ServerSendTargetData(FNexusActionDefHandle Handle, FNexusPredictionTag PrimaryPredictionTag, FNexusPredictionTag PredictionTag, FNexusTargetDataHandle TargetDataHandle);
	void CallOrAddTargetDataDelegate(FNexusActionDefHandle Handle, FNexusPredictionTag PrimaryPredictionTag, FOnNexusTargetDataSetSignature::FDelegate&& Delegate);


private:
	struct FNexusRepDataDelegate
	{
		FNexusRepDataDelegate(const FNexusPredictionTag& InPredictionTag)
			: PredictionTag(InPredictionTag)
		{
		}


		FNexusPredictionTag PredictionTag;
	};

	struct FNexusNetSyncDelegate : public FNexusRepDataDelegate
	{
		FNexusNetSyncDelegate(const FNexusPredictionTag& InPredictionTag)
			: FNexusRepDataDelegate(InPredictionTag)
		{
		}


		FNexusNetSyncDelegate(const FNexusPredictionTag& InPredictionTag, FSimpleMulticastDelegate::FDelegate&& InDelegate)
			: FNexusRepDataDelegate(InPredictionTag)
		{
			OnSyncDelegate.Add(MoveTemp(InDelegate));
		}

		FSimpleMulticastDelegate OnSyncDelegate;
	};

	struct FNexusTargetDataDelegate : public FNexusRepDataDelegate
	{
		FNexusTargetDataDelegate(const FNexusPredictionTag& InPredictionTag, const FNexusTargetDataHandle& InTargetDataHandle)
			: FNexusRepDataDelegate(InPredictionTag)
			  , TargetDataHandle(InTargetDataHandle)
		{
		}

		FNexusTargetDataDelegate(FOnNexusTargetDataSetSignature::FDelegate&& InDelegate)
			: FNexusRepDataDelegate(FNexusPredictionTag()),
			  TargetDataHandle()
		{
			OnSetDelegate.Add(MoveTemp(InDelegate));
		}


		FOnNexusTargetDataSetSignature OnSetDelegate;
		FNexusTargetDataHandle TargetDataHandle;
	};

private:
	UPROPERTY(Replicated)
	FNexusPredictionTagContainer NetPredictionTags;

	FNexusPredictionTag CurrentPredictionTag;


	TMap<FNexusRepDataKey, FNexusNetSyncDelegate> NetSyncPointDelegates;
	TMap<FNexusRepDataKey, FNexusTargetDataDelegate> TargetDataDelegates;
};
