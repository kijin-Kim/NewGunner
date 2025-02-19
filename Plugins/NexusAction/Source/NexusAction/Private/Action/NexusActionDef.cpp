// Fill out your copyright notice in the Description page of Project Settings.


#include "Action/NexusActionDef.h"
#include "Action/NexusAction.h"


FNexusActionDef::FNexusActionDef(): SourceObject(nullptr), ActionClass(nullptr)
{
	Handle.GenerateNewHandle();
}

FNexusActionDef::FNexusActionDef(UObject* InSourceObject, TSubclassOf<UNexusAction> InActionClass): SourceObject(InSourceObject), ActionClass(InActionClass)
{
	Handle.GenerateNewHandle();
}

bool FNexusActionDef::operator==(const FNexusActionDef& Other) const
{
	return Handle == Other.Handle;
}

bool FNexusActionDef::operator!=(const FNexusActionDef& Other) const
{
	return !(*this == Other);
}

void FNexusActionDef::PostReplicatedAdd(const FNexusActionDefContainer& InArraySerializer)
{
	InArraySerializer.OnAdded(*this);
}

void FNexusActionDef::PreReplicatedRemove(const FNexusActionDefContainer& InArraySerializer)
{
	InArraySerializer.OnRemoved(*this);
}

void FNexusActionDefContainer::AuthAdd(const FNexusActionDef& ActionDef)
{
	int32 Index = Items.Add(ActionDef);
	MarkItemDirty(Items[Index]);
}

void FNexusActionDefContainer::AuthRemove(const FNexusActionDefHandle& Handle)
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

void FNexusActionDefContainer::AuthRemoveAll()
{
	Items.Empty();
	MarkArrayDirty();
}

FNexusActionDef* FNexusActionDefContainer::FindActionDefByHandle(const FNexusActionDefHandle& Handle)
{
	for (FNexusActionDef& Item : Items)
	{
		if (Item.Handle == Handle)
		{
			return &Item;
		}
	}
	return nullptr;
}

bool FNexusActionDefContainer::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FNexusActionDef, FNexusActionDefContainer>(Items, DeltaParms, *this);
}

void FNexusActionDefContainer::OnAdded(FNexusActionDef& ActionDef) const
{
	if (OnActionDefAddedDelegate.IsBound())
	{
		OnActionDefAddedDelegate.Execute(ActionDef);
	}
}

void FNexusActionDefContainer::OnRemoved(FNexusActionDef& ActionDef) const
{
	if (OnActionDefRemovedDelegate.IsBound())
	{
		OnActionDefRemovedDelegate.Execute(ActionDef);
	}
}
