// Fill out your copyright notice in the Description page of Project Settings.


#include "Cue/NexusCueNetworkProxyInterface.h"


void INexusCueNetworkProxyInterface::CallNetMulticastTriggerCue(TSubclassOf<ANexusCue> CueClass, FNexusPredictionTag PredictionTag, const FNexusCueParameters& CueParameters)
{
	NetMulticastTriggerCue(CueClass, PredictionTag, CueParameters);
}
