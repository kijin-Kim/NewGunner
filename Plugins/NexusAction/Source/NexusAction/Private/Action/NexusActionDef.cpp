// Fill out your copyright notice in the Description page of Project Settings.


#include "Action/NexusActionDef.h"

#include "NexusLog.h"
#include "Action/NexusAction.h"


FNexusActionDef::FNexusActionDef()
	: ActionClass(nullptr),
	  SourceObject(nullptr)
{
	Handle.GenerateNewHandle();
}

FNexusActionDef::FNexusActionDef(UObject* InSourceObject, TSubclassOf<UNexusAction> InActionClass)
	: ActionClass(InActionClass),
	  SourceObject(InSourceObject)

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
	return FString::Printf(TEXT("ActionDef={ActionClass=%s, Handle=%s, SourceObject=%s}"), *ActionClass->GetName(), *Handle.ToString(), SourceObject ? *SourceObject->GetName() : TEXT(""));
}

void FNexusActionDef::PreReplicatedRemove(const FNexusActionDefContainer& InArraySerializer)
{
	InArraySerializer.OnRemoved(*this);
}

FNexusActionDefContainer::~FNexusActionDefContainer()
{
	Items.Empty();
}

void FNexusActionDefContainer::Init()
{
	bInitialized = true;
	FlushPendingAdds();
}

void FNexusActionDefContainer::AuthAdd(const FNexusActionDef& ActionDef)
{
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

const FNexusActionDef* FNexusActionDefContainer::FindActionDefByHandle(const FNexusActionDefHandle& Handle) const
{
	check(Handle.IsValid());
	for (const FNexusActionDef& Item : Items)
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


bool FNexusActionDefContainer::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FNexusActionDef, FNexusActionDefContainer>(Items, DeltaParms, *this);
}

void FNexusActionDefContainer::OnAdded(FNexusActionDef& ActionDef) const
{
	OnActionDefAddedDelegate.ExecuteIfBound(ActionDef);
}

void FNexusActionDefContainer::OnRemoved(FNexusActionDef& ActionDef) const
{
	OnActionDefRemovedDelegate.ExecuteIfBound(ActionDef);
}

void FNexusActionDefContainer::FlushPendingAdds()
{
	for (const auto& [OuterIndex, GuidRefeMap] : GuidReferencesMap_StructDelta)
	{
		for (const auto& [Index, GuidRefs] : GuidRefeMap)
		{
			if (GuidRefs.GetUnmappedGUIDs().Num() > 0)
			{
				return;
			}
		}
	}
	
	check(bInitialized);
	for (FNexusActionDef& Item : Items)
	{
		if (!Item.bIsAdded)
		{
			Item.bIsAdded = true;
			OnAdded(Item);
		}
	}
}


void FNexusActionDefContainer::PostReplicatedReceive(const FFastArraySerializer::FPostReplicatedReceiveParameters& Parameters)
{
	// 다음 상황에서 SourceObject가 매핑되지 않을 수 있음
	// 1. SourceObject가 리플리케이트 될 때까지 기다린 후 액션을 최종적으로 추가
	// 2. SourceObject가 같은 프레임에 도착하나, 리플리케이트 순서가 빠를 경우,
	// 3. SourceObject가 클라이언트에서 아직 소환되지 않은 액터일 경우

	// 이를 위해 PostReplicatedReceive를 통해 SourceObject가 완전히 매핑된 후 액션을 추가함
	if (!Parameters.bHasMoreUnmappedReferences && bInitialized)
	{
		FlushPendingAdds();
		UE_LOG(LogNexusAction, VeryVerbose, TEXT("액션 추가 플러시"));
	}
	else
	{
		UE_LOG(LogNexusAction, VeryVerbose, TEXT("매핑되지 않은 레퍼런스 존재"));
	}
}
