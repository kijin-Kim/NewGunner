// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NexusCue.h"
#include "Prediction/NexusPrediction.h"
#include "UObject/Interface.h"
#include "NexusCueNetworkProxyInterface.generated.h"



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
	void CallNetMulticastTriggerCue(TSubclassOf<ANexusCue> CueClass, FNexusPredictionTag PredictionTag, const FNexusCueParameters& CueParameters);
	virtual void NetMulticastTriggerCue(TSubclassOf<ANexusCue> CueClass, FNexusPredictionTag PredictionTag, const FNexusCueParameters& CueParameters) = 0;
};
