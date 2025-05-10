// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NexusAgentInfo.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct NEXUSACTION_API FNexusAgentInfo
{
	GENERATED_BODY()

	FNexusAgentInfo() = default;
	FNexusAgentInfo(AActor* InOwnerActor, AActor* InAgentActor)
	{
		Init(InOwnerActor, InAgentActor);
	}

	void Init(AActor* InOwnerActor, AActor* InAgentActor);

	bool operator==(const FNexusAgentInfo& Other) const = default;
	

	bool IsLocallyPlayerControlled() const;
	bool IsLocallyControlled() const;
	bool IsOwnerActorAuthoritative() const;


	AController* GetController() const;
	AActor* GetAgentActor() const;
	AActor* GetOwnerActor() const;

protected:
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AController> Controller;
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> AgentActor;
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> OwnerActor;
};
