// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionDefinition.h"
#include "GunnerAction.h"


FGunnerActionDefinition::FGunnerActionDefinition(): ActionClass(nullptr)
{
	Handle.GenerateNewHandle();
}

FGunnerActionDefinition::FGunnerActionDefinition(UObject* InSourceObject, TSubclassOf<UGunnerAction> InActionClass): ActionClass(InActionClass)
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

void FGunnerActionDefinition::PostReplicatedAdd(const FGunnerActionDefinitionArray& InArraySerializer)
{
	InArraySerializer.OnAdded(*this);
}

void FGunnerActionDefinition::PreReplicatedRemove(const FGunnerActionDefinitionArray& InArraySerializer)
{
	InArraySerializer.OnRemoved(*this);
}

void FGunnerActionDefinitionArray::AuthAdd(const FGunnerActionDefinition& ActionDefinition)
{
	int32 Index = Items.Add(ActionDefinition);
	MarkItemDirty(Items[Index]);
}

void FGunnerActionDefinitionArray::AuthRemove(const FGunnerActionDefinitionHandle& Handle)
{
	for (int32 i = 0; i < Items.Num(); i++)
	{
		if (Items[i].Handle == Handle)
		{
			Items.RemoveAt(i);
			break;
		}
	}
	MarkArrayDirty();
}

void FGunnerActionDefinitionArray::AuthRemoveAll()
{
	Items.Empty();
	MarkArrayDirty();
}

FGunnerActionDefinition* FGunnerActionDefinitionArray::FindActionDefinitionByHandle(const FGunnerActionDefinitionHandle& Handle)
{
	for (FGunnerActionDefinition& Item : Items)
	{
		if (Item.Handle == Handle)
		{
			return &Item;
		}
	}
	return nullptr;
}

bool FGunnerActionDefinitionArray::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FGunnerActionDefinition, FGunnerActionDefinitionArray>(Items, DeltaParms, *this);
}

void FGunnerActionDefinitionArray::OnAdded(FGunnerActionDefinition& ActionDefinition) const
{
	if (OnActionDefinitionAddedDelegate.IsBound())
	{
		OnActionDefinitionAddedDelegate.Execute(ActionDefinition);
	}
}

void FGunnerActionDefinitionArray::OnRemoved(FGunnerActionDefinition& ActionDefinition) const
{
	if (OnActionDefinitionRemovedDelegate.IsBound())
	{
		OnActionDefinitionRemovedDelegate.Execute(ActionDefinition);
	}
}
