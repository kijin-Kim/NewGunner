// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Cue/NexusCue.h"
#include "Cue/NexusCueNetworkProxyInterface.h"
#include "NexusCueComponent.generated.h"


struct FNexusAgentInfo;




UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NEXUSACTION_API UNexusCueComponent : public UActorComponent, public INexusCueNetworkProxyInterface
{
	GENERATED_BODY()

public:
	UNexusCueComponent();
	void Init(TSharedPtr<FNexusAgentInfo> InAgentInfo);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void TriggerCue(const FNexusTriggerCueParams& TriggerCueParams);
	void AuthEndCue(FNexusLoopingCueHandle CueHandle);

	void SimTriggerCue(const FNexusTriggerCueParams& TriggerCueParams, FNexusLoopingCueHandle ServerCueHandle);
	void RemoveAllLoopingCues();

private:
	
	UFUNCTION(NetMulticast, Unreliable)
	virtual void NetMulticastTriggerCue(const FNexusTriggerCueParams& TriggerCueParams, FNexusLoopingCueHandle ServerCueHandle) override;
	INexusCueNetworkProxyInterface* GetCueNetworkProxyInterface();
	
	
	void OnCueAdded(FNexusLoopingCue& NexusLoopingCue);
	void OnCueRemoved(FNexusLoopingCue& NexusLoopingCue);
	

private:
	TSharedPtr<FNexusAgentInfo> AgentInfo;
	UPROPERTY(Replicated)
	FNexusLoopingCueContainer LoopingCues;

};
