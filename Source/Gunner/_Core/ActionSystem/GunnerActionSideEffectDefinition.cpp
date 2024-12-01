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
	return Handle == Other.Handle && SideEffectClass == Other.SideEffectClass;
}

bool FGunnerActionSideEffectDefinition::operator!=(const FGunnerActionSideEffectDefinition& Other) const
{
	return !(*this == Other);
}

void FGunnerActionSideEffectDefinitionItem::PostReplicatedAdd(const FGunnerActionSideEffectDefinitionArray& InArraySerializer)
{
	InArraySerializer.OnAdded(SideEffectDefinition, PredictionHandle);
}

void FGunnerActionSideEffectDefinitionItem::PreReplicatedRemove(const FGunnerActionSideEffectDefinitionArray& InArraySerializer)
{
	InArraySerializer.OnRemoved(SideEffectDefinition.Handle);
}

void FGunnerActionSideEffectDefinitionItem::PostReplicatedChange(const FGunnerActionSideEffectDefinitionArray& InArraySerializer)
{
	UE_DEBUG_BREAK();
}

void FGunnerActionSideEffectDefinitionArray::Add(const FGunnerActionSideEffectDefinition& SideEffectDefinition, FGunnerActionNetPredictionHandle PredictionHandle, bool bHasAuthority)
{
	FGunnerActionSideEffectDefinitionItem NewItem;
	NewItem.SideEffectDefinition.Handle = SideEffectDefinition.Handle;
	NewItem.SideEffectDefinition.SideEffectClass = SideEffectDefinition.SideEffectClass;
	NewItem.SideEffectDefinition.SideEffectCDO = SideEffectDefinition.SideEffectCDO;
	NewItem.PredictionHandle = PredictionHandle;

	int Index = Items.Add(NewItem);
	if (bHasAuthority)
	{
		MarkItemDirty(Items[Index]);
	}
	else
	{
		MarkArrayDirty();
	}

	OnAdded(SideEffectDefinition, PredictionHandle);
	
	if (SideEffectDefinition.SideEffectCDO->DurationType == ESideEffectDurationType::Instant && bHasAuthority)
	{
		Remove(SideEffectDefinition.Handle);
	}
}

void FGunnerActionSideEffectDefinitionArray::OnAdded(const FGunnerActionSideEffectDefinition& SideEffectDefinition, FGunnerActionNetPredictionHandle PredictionHandle) const
{
	OnSideEffectDefinitionAddedDelegate.ExecuteIfBound(SideEffectDefinition, PredictionHandle);
}

void FGunnerActionSideEffectDefinitionArray::Remove(const FGunnerActionSideEffectDefinitionHandle& SideEffectDefinitionHandle)
{
	int32 Removed = Items.RemoveAll([SideEffectDefinitionHandle](const FGunnerActionSideEffectDefinitionItem& SideEffectDefinitionItem)
	{
		return SideEffectDefinitionItem.SideEffectDefinition.Handle == SideEffectDefinitionHandle;
	});
	check(Removed != 0);
	MarkArrayDirty();
	OnRemoved(SideEffectDefinitionHandle);
}

void FGunnerActionSideEffectDefinitionArray::OnRemoved(const FGunnerActionSideEffectDefinitionHandle& SideEffectDefinitionHandle) const
{
	OnSideEffectDefinitionRemovedDelegate.ExecuteIfBound(SideEffectDefinitionHandle);
}

bool FGunnerActionSideEffectDefinitionArray::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FGunnerActionSideEffectDefinitionItem, FGunnerActionSideEffectDefinitionArray>(Items, DeltaParms, *this);
}
