// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "NexusCheatManager.generated.h"

/**
 * 
 */
UCLASS()
class NEXUSACTION_API UNexusCheatManager : public UCheatManager
{
	GENERATED_BODY()
	
public:
	UFUNCTION(Exec)
	void SendEventToSelf(const FString& EventTagString);
	UFUNCTION(Exec)
	void SendEventToTarget(const FString& EventTagString);
	
};

