// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Action/NexusAgentInfo.h"
#include "Components/ActorComponent.h"
#include "NexusAgentBoundComponent.generated.h"



UCLASS(Abstract)
class NEXUSACTION_API UNexusAgentBoundComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void Setup(TSharedPtr<FNexusAgentInfo> InAgentInfo)
	{
		AgentInfo = InAgentInfo;
	}

	TSharedPtr<FNexusAgentInfo> GetAgentInfo() const { return AgentInfo; }
	AActor* GetAgentActor() const { return AgentInfo.IsValid() ? AgentInfo->AgentActor.Get() : nullptr; }
	AActor* GetOwnerActor() const { return AgentInfo.IsValid() ? AgentInfo->OwnerActor.Get() : nullptr; }
	AController* GetController() const { return AgentInfo.IsValid() ? AgentInfo->Controller.Get() : nullptr; }

protected:
	TSharedPtr<FNexusAgentInfo> AgentInfo;
};
