// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NexusAgentBoundComponent.h"
#include "Components/ActorComponent.h"
#include "Prediction/NexusDataReplication.h"
#include "Prediction/NexusPrediction.h"
#include "TargetData/NexusTargetData.h"
#include "NexusPredictionComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NEXUSACTION_API UNexusPredictionComponent : public UNexusAgentBoundComponent
{
	GENERATED_BODY()

public:
	UNexusPredictionComponent();
	virtual void Setup(TSharedPtr<FNexusAgentInfo> InAgentInfo) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void ReplicateNetPredictionTag(const FNexusPredictionTag& PredictionTag);
	void SetCurrentPredictionTag(const FNexusPredictionTag& NewTag);
	FNexusPredictionTag GetCurrentPredictionTag() const;

	UFUNCTION(Server, Reliable)
	void ServerSendNetSyncPoint(const FNexusActionDefHandle& Handle, FNexusPredictionTag PrimaryPredictionTag, FNexusPredictionTag PredictionTag);
	void AuthCallOrAddNetsyncPointDelegate(const FNexusActionDefHandle& Handle, FNexusPredictionTag PrimaryPredictionTag, FSimpleMulticastDelegate::FDelegate&& Delegate);

	UFUNCTION(Server, Reliable)
	void ServerSendTargetData(const FNexusActionDefHandle& Handle, FNexusPredictionTag PrimaryPredictionTag, FNexusPredictionTag PredictionTag, const FNexusTargetDataHandle& TargetDataHandle);
	void AuthCallOrAddTargetDataDelegate(const FNexusActionDefHandle& Handle, FNexusPredictionTag PrimaryPredictionTag, FOnNexusTargetDataSetSignature::FDelegate&& Delegate);
	void AuthClearAllReplicationDelegates(const FNexusActionDefHandle& Handle, FNexusPredictionTag PrimaryPredictionTag);

private:
	struct FNexusNetSyncDelegate
	{
		void Reset()
		{
			PredictionTag = FNexusPredictionTag();
			OnSyncDelegate.Clear();
		}

		FNexusPredictionTag PredictionTag;
		FSimpleMulticastDelegate OnSyncDelegate;
	};

	struct FNexusTargetDataDelegate
	{
		void Reset()
		{
			PredictionTag = FNexusPredictionTag();
			OnSetDelegate.Clear();
			TargetDataHandle = FNexusTargetDataHandle();
		}

		FNexusPredictionTag PredictionTag;
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
