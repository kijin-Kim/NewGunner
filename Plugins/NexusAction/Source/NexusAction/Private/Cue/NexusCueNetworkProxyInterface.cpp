// Fill out your copyright notice in the Description page of Project Settings.


#include "Cue/NexusCueNetworkProxyInterface.h"

void INexusCueNetworkProxyInterface::CallNetMulticastTriggerCue(TSubclassOf<UNexusCue> CueClass, FNexusTargetDataHandle TargetDataHandle, FNexusPredictionTag PredictionTag)
{
	NetMulticastTriggerCue(CueClass, TargetDataHandle, PredictionTag);
}
