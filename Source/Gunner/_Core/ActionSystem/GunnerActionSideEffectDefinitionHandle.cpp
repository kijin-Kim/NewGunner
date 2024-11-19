// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionSideEffectDefinitionHandle.h"

void FGunnerActionSideEffectDefinitionHandle::GenerateNewHandle()
{
	static int32 HandleCounter = 1;
	Handle = HandleCounter++;
}
