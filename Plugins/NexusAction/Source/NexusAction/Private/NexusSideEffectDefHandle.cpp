// Fill out your copyright notice in the Description page of Project Settings.


#include "NexusSideEffectDefHandle.h"

void FNexusSideEffectDefHandle::GenerateNewHandle()
{
	static int32 HandleCounter = 1;
	Handle = HandleCounter++;
}
