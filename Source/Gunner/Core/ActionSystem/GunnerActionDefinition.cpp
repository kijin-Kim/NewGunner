// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionDefinition.h"
#include "GunnerAction.h"

void FGunnerActionDefinitionHandle::GenerateNewHandle()
{
	static int32 HandleCounter = 1;
	Handle = HandleCounter++;
}

FGunnerActionDefinition::FGunnerActionDefinition(): SourceObject(nullptr), ActionClass(nullptr), ActionCDO(ActionClass ? ActionClass.GetDefaultObject() : nullptr)
{
	Handle.GenerateNewHandle();
}

FGunnerActionDefinition::FGunnerActionDefinition(UObject* InSourceObject, TSubclassOf<UGunnerAction> InActionClass): SourceObject(InSourceObject), ActionClass(InActionClass), ActionCDO(ActionClass ? ActionClass.GetDefaultObject() : nullptr)
{
	Handle.GenerateNewHandle();
}
