// Fill out your copyright notice in the Description page of Project Settings.


#include "Cue/NexusCueNetworkProxyInterface.h"

void INexusCueNetworkProxyInterface::CallNetMulticastTriggerCue(const FNexusTriggerCueParams& TriggerCueParams, FNexusLoopingCueHandle CueHandle)
{
	NetMulticastTriggerCue(TriggerCueParams, CueHandle);
}
