// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionDefinition.h"
#include "GunnerAction.h"


FGunnerActionDefinition::FGunnerActionDefinition(): SourceObject(nullptr), ActionClass(nullptr), ActionCDO(ActionClass ? ActionClass.GetDefaultObject() : nullptr)
{
	Handle.GenerateNewHandle();
}

FGunnerActionDefinition::FGunnerActionDefinition(UObject* InSourceObject, TSubclassOf<UGunnerAction> InActionClass): SourceObject(InSourceObject), ActionClass(InActionClass), ActionCDO(ActionClass ? ActionClass.GetDefaultObject() : nullptr)
{
	Handle.GenerateNewHandle();
}

bool FGunnerActionDefinition::operator==(const FGunnerActionDefinition& Other) const
{
	return Handle == Other.Handle;
}

bool FGunnerActionDefinition::operator!=(const FGunnerActionDefinition& Other) const
{
	return !(*this == Other);
}
