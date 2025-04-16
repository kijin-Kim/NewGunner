// Fill out your copyright notice in the Description page of Project Settings.


#include "SideEffect/NexusSideEffectInstanceHandle.h"

void FNexusSideEffectInstanceHandle::GenerateNewHandle()
{
	static int32 HandleCounter = 1;
	Handle = HandleCounter++;
}
