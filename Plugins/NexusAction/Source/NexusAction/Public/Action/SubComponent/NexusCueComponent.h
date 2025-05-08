// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NexusAgentBoundComponent.h"
#include "Components/ActorComponent.h"
#include "Cue/NexusCue.h"
#include "Cue/NexusCueNetworkProxyInterface.h"
#include "NexusCueComponent.generated.h"


struct FNexusAgentInfo;


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NEXUSACTION_API UNexusCueComponent : public UNexusAgentBoundComponent, public INexusCueNetworkProxyInterface
{
	GENERATED_BODY()

public:
	UNexusCueComponent();
	virtual void Setup(TSharedPtr<FNexusAgentInfo> InAgentInfo) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void TriggerCue(TSubclassOf<ANexusCue> CueClass, FNexusPredictionTag PredictionTag, const FNexusCueParameters& CueParameters);
	void AuthEndCue(TSubclassOf<ANexusCue> CueClass);
	void InternalAuthEndCue(FNexusLoopingCueHandle LoopingCueHandle);

	void SimTriggerCue(TSubclassOf<ANexusCue> CueClass, FNexusPredictionTag PredictionTag, const FNexusCueParameters& CueParameters);
	void AuthRemoveAllLoopingCues();

	const TMap<TSubclassOf<ANexusCue>, TObjectPtr<ANexusCue>>& GetLocalLoopingCueActors() const { return LocalLoopingCueActors; }
	const TMap<TSubclassOf<ANexusCue>, int32>& GetLocalLoopingCueActorsCount() const { return LocalLoopingCueActorsCount; }

private:
	UFUNCTION(NetMulticast, Unreliable)
	virtual void NetMulticastTriggerCue(TSubclassOf<ANexusCue> CueClass, FNexusPredictionTag PredictionTag, const FNexusCueParameters& CueParameters) override;
	INexusCueNetworkProxyInterface* GetCueNetworkProxyInterface();


	void OnCueAdded(const FNexusLoopingCue& NexusLoopingCue);
	void OnCueRemoved(const FNexusLoopingCue& LoopingCue);


	ANexusCue* FindOrCreateLoopingCueActor(TSubclassOf<ANexusCue> CueClass);


private:
	UPROPERTY(Replicated)
	FNexusLoopingCueContainer LoopingCues;
	UPROPERTY()
	TMap<TSubclassOf<ANexusCue>, TObjectPtr<ANexusCue>> LocalLoopingCueActors;
	TMap<TSubclassOf<ANexusCue>, int32> LocalLoopingCueActorsCount;
};
