// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionDefinitionHandle.h"


// Add default functionality here for any IGunnerActionDefinitionHandle functions that are not pure virtual.
void FGunnerActionDefinitionHandle::GenerateNewHandle()
{
	static int32 HandleCounter = 1;
	Handle = HandleCounter++;
}

