// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerActionAgentInfo.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FGunnerActionAgentInfo
{
	GENERATED_BODY()
	void Init(AActor* InOwnerActor, AActor* InAgentActor);

	bool operator==(const FGunnerActionAgentInfo& Other) const = default;

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AController> Controller;
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> AgentActor;
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> OwnerActor;
	
	bool IsLocallyPlayerControlled() const;
	bool IsLocallyControlled() const;
	bool IsOwnerActorAuthoritative() const;

	
	
};
