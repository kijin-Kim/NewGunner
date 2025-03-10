// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NexusPrediction.h"
#include "TargetData/NexusTargetData.h"
#include "UObject/Interface.h"
#include "NexusCueNetworkProxyInterface.generated.h"

class UNexusCue;
class UNexusAction;
// This class does not need to be modified.
UINTERFACE()
class UNexusCueNetworkProxyInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class NEXUSACTION_API INexusCueNetworkProxyInterface
{
	GENERATED_BODY()

public:
	
	void CallNetMulticastTriggerCue(TSubclassOf<UNexusCue> CueClass, FNexusTargetDataHandle TargetDataHandle, FNexusPredictionTag PredictionTag);
	virtual void NetMulticastTriggerCue(TSubclassOf<UNexusCue> CueClass, FNexusTargetDataHandle TargetDataHandle, FNexusPredictionTag PredictionTag) = 0;
};
