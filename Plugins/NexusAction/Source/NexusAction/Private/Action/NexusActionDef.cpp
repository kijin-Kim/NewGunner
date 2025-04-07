// Fill out your copyright notice in the Description page of Project Settings.


#include "Action/NexusActionDef.h"

#include "NexusLog.h"
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
	return Handle == Other.Handle && SourceObject == Other.SourceObject && ActionClass == Other.ActionClass;
}

bool FNexusActionDef::operator!=(const FNexusActionDef& Other) const
{
	return !(*this == Other);
}

FString FNexusActionDef::ToString() const
{
	return FString::Printf(TEXT("ActionClass: %s, Handle: %s, SourceObject: %s"), *ActionClass->GetName(), *Handle.ToString(), SourceObject.IsValid() ? *SourceObject->GetName() : TEXT(""));
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
	if (HasSameActionClassAndSourceObject(ActionDef))
	{
		UE_LOG(LogNexusAction, Verbose, TEXT("액션 데피니션 [ActionClass: %s, SourceObject: %s]가 이미 추가 되었습니다."), *ActionDef.ActionClass->GetName(), ActionDef.SourceObject.IsValid() ? *ActionDef.SourceObject->GetName() : TEXT(""));
		return;
	}
	int32 Index = Items.Add(ActionDef);
	MarkItemDirty(Items[Index]);
	OnAdded(Items[Index]);
}

void FNexusActionDefContainer::AuthRemove(const FNexusActionDefHandle& Handle)
{
	for (int32 i = 0; i < Items.Num(); i++)
	{
		if (Items[i].Handle == Handle)
		{
			OnRemoved(Items[i]);
			Items.RemoveAt(i);
			break;
		}
	}
	MarkArrayDirty();
}

void FNexusActionDefContainer::AuthRemoveAll()
{
	for (int32 i = 0; i < Items.Num(); i++)
	{
		OnRemoved(Items[i]);
	}
	Items.Empty();
	MarkArrayDirty();
}

FNexusActionDef* FNexusActionDefContainer::FindActionDefByHandle(FNexusActionDefHandle Handle)
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

bool FNexusActionDefContainer::HasSameActionClassAndSourceObject(const FNexusActionDef& ActionDef) const
{
	return FindActionDefHandle(ActionDef.ActionClass, ActionDef.SourceObject.Get()).IsValid();
}

FNexusActionDefHandle FNexusActionDefContainer::FindActionDefHandle(TSubclassOf<UNexusAction> ActionClass, UObject* SourceObject) const
{
	for (const FNexusActionDef& Item : Items)
	{
		if (Item.ActionClass == ActionClass && Item.SourceObject == SourceObject)
		{
			return Item.Handle;
		}
	}
	return FNexusActionDefHandle();
}

TArray<FNexusActionDef> FNexusActionDefContainer::GetAllTriggeringActionDefs() const
{
	TArray<FNexusActionDef> TriggeringActionDefs;
	for (const FNexusActionDef& Item : Items)
	{
		if (Item.ActionInstance && Item.ActionInstance->IsTriggering())
		{
			TriggeringActionDefs.Add(Item);
		}
	}
	return TriggeringActionDefs;
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
