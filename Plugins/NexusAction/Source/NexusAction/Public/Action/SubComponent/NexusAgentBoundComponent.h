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
	AActor* GetAgentActor() const { return AgentInfo.IsValid() ? AgentInfo->GetAgentActor() : nullptr; }
	AActor* GetOwnerActor() const { return AgentInfo.IsValid() ? AgentInfo->GetOwnerActor() : nullptr; }
	AController* GetController() const { return AgentInfo.IsValid() ? AgentInfo->GetController() : nullptr; }
	bool IsLocallyPlayerControlled() const { return AgentInfo.IsValid() ? AgentInfo->IsLocallyPlayerControlled() : false; }
	bool IsLocallyControlled() const { return AgentInfo.IsValid() ? AgentInfo->IsLocallyControlled() : false; }
	bool IsOwnerActorAuthoritative() const { return AgentInfo.IsValid() ? AgentInfo->IsOwnerActorAuthoritative() : false; }

protected:
	TSharedPtr<FNexusAgentInfo> AgentInfo;
};
