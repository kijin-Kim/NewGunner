// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionSideEffectDefinition.h"
#include "GunnerActionSideEffect.h"

FGunnerActionSideEffectDefinition::FGunnerActionSideEffectDefinition(): SideEffectClass(nullptr), SideEffectCDO(SideEffectClass ? SideEffectClass.GetDefaultObject() : nullptr)
{
	Handle.GenerateNewHandle();
}

FGunnerActionSideEffectDefinition::FGunnerActionSideEffectDefinition(TSubclassOf<UGunnerActionSideEffect> InActionClass): SideEffectClass(InActionClass), SideEffectCDO(SideEffectClass ? SideEffectClass.GetDefaultObject() : nullptr)
{
	Handle.GenerateNewHandle();
}

bool FGunnerActionSideEffectDefinition::operator==(const FGunnerActionSideEffectDefinition& Other) const
{
	return Handle == Other.Handle;
}

bool FGunnerActionSideEffectDefinition::operator!=(const FGunnerActionSideEffectDefinition& Other) const
{
	return !(*this == Other);
}
