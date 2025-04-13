// Fill out your copyright notice in the Description page of Project Settings.


#include "Cue/NexusCueNetworkProxyInterface.h"

void INexusCueNetworkProxyInterface::CallNetMulticastTriggerCue(TSubclassOf<ANexusCue> CueClass, FNexusTargetDataHandle TargetDataHandle, FNexusPredictionTag PredictionTag,  FNexusLoopingCueHandle CueHandle)
{
	NetMulticastTriggerCue(CueClass, TargetDataHandle, PredictionTag, CueHandle);
}
