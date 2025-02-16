// Fill out your copyright notice in the Description page of Project Settings.


#include "Action/NexusActionDefHandle.h"


void FNexusActionDefHandle::GenerateNewHandle()
{
	static int32 HandleCounter = 1;
	Handle = HandleCounter++;
}

