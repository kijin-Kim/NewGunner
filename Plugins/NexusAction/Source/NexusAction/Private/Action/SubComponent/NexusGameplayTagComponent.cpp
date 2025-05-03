// Fill out your copyright notice in the Description page of Project Settings.


#include "Action/SubComponent/NexusGameplayTagComponent.h"

#include "NexusLog.h"
#include "Net/UnrealNetwork.h"


void FNexusGameplayTagCountContainer::AddTagCount(const FGameplayTag& InTag, int32 InCount)
{
	int32 Index = Items.Find(FNexusGameplayTagCount{InTag});
	if (Index == INDEX_NONE)
	{
		Items.Emplace(FNexusGameplayTagCount{InTag, 0});
		Index = Items.Num() - 1;
	}

	Items[Index].Count += InCount;

	if (bHasAuthority)
	{
		MarkItemDirty(Items[Index]);
	}
}


void FNexusGameplayTagCountContainer::SubtractTagCount(const FGameplayTag& InTag, int32 InCount)
{
	int32 Index = Items.Find(FNexusGameplayTagCount{InTag});
	if (Index == INDEX_NONE)
	{
		Items.Emplace(FNexusGameplayTagCount{InTag, 0});
		Index = Items.Num() - 1;
	}

	Items[Index].Count -= InCount;

	if (bHasAuthority)
	{
		MarkItemDirty(Items[Index]);
	}
}

void FNexusGameplayTagCountContainer::SetTagCount(const FGameplayTag& InTag, int32 InCount)
{
	int32 Index = Items.Find(FNexusGameplayTagCount{InTag});
	if (Index == INDEX_NONE)
	{
		Items.Emplace(FNexusGameplayTagCount{InTag, 0});
		Index = Items.Num() - 1;
	}

	Items[Index].Count = InCount;

	if (bHasAuthority)
	{
		MarkItemDirty(Items[Index]);
	}
}

int32 FNexusGameplayTagCountContainer::GetTagCount(const FGameplayTag& InTag) const
{
	int32 Index = Items.Find(FNexusGameplayTagCount{InTag});
	if (Index == INDEX_NONE)
	{
		return 0;
	}
	return Items[Index].Count;
}

void FNexusGameplayTagCountContainer::RemoveAllTagCounts()
{
	Items.Empty();
	MarkArrayDirty();
}

bool FNexusGameplayTagCountContainer::Contains(const FGameplayTag& InTag) const
{
	return Items.Contains(FNexusGameplayTagCount{InTag});
}

// Sets default values for this component's properties
UNexusGameplayTagComponent::UNexusGameplayTagComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UNexusGameplayTagComponent::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);
	StaticTagCountContainer.Init(GetOwner()->HasAuthority());
	DynamicTagCountContainer.Init(GetOwner()->HasAuthority());
}

void UNexusGameplayTagComponent::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer.Reset();
	for (const FNexusGameplayTagCount& TagCount : DynamicTagCountContainer.Items)
	{
		if (TagCount.Count > 0)
		{
			TagContainer.AddTag(TagCount.Tag);
		}
	}
}

void UNexusGameplayTagComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UNexusGameplayTagComponent, StaticTagCountContainer, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UNexusGameplayTagComponent, DynamicTagCountContainer, COND_SimulatedOnly);
}

void UNexusGameplayTagComponent::EvaluateTagCounts()
{
	if (bIsDirty)
	{
		bIsDirty = false;
		DynamicTagCountDeltas = DynamicTagCountDeltas.FilterByPredicate([](const auto& Pair)
		{
			return Pair.Value != 0;
		});

		TArray<FNexusGameplayTagCount> OldDynamicTagCountMapItems = DynamicTagCountContainer.Items;
		DynamicTagCountContainer.RemoveAllTagCounts();
		for (const auto& [Tag,Delta] : DynamicTagCountDeltas)
		{
			int32 BaseCount = StaticTagCountContainer.GetTagCount(Tag);
			DynamicTagCountContainer.SetTagCount(Tag, BaseCount + Delta);
		}

		OnCountMapEvaluated(OldDynamicTagCountMapItems);
	}
}

void UNexusGameplayTagComponent::PushDynamicTag(const FGameplayTag& Tag)
{
	bIsDirty = true;
	check(Tag.IsValid());
	NX_LOG_SUB(GetAgentActor(), LogNexusGameplayTag, Verbose, TEXT("다이내믹 태그 푸시: GameplayTag=%s"), *Tag.ToString());
	DynamicTagCountDeltas.FindOrAdd(Tag)++;
}

void UNexusGameplayTagComponent::PopDynamicTag(const FGameplayTag& Tag)
{
	bIsDirty = true;
	check(Tag.IsValid());
	NX_LOG_SUB(GetAgentActor(), LogNexusGameplayTag, Verbose, TEXT("다이내믹 태그 팝: GameplayTag=%s"), *Tag.ToString());
	DynamicTagCountDeltas.FindOrAdd(Tag)--;
}

void UNexusGameplayTagComponent::PushStaticTag(const FGameplayTag& Tag)
{
	bIsDirty = true;
	check(Tag.IsValid());
	NX_LOG_SUB(GetAgentActor(), LogNexusGameplayTag, Verbose, TEXT("태그 푸시: GameplayTag=%s"), *Tag.ToString());
	StaticTagCountContainer.AddTagCount(Tag, 1);
}

void UNexusGameplayTagComponent::PopStaticTag(const FGameplayTag& Tag)
{
	bIsDirty = true;
	check(Tag.IsValid());
	NX_LOG_SUB(GetAgentActor(), LogNexusGameplayTag, Verbose, TEXT("태그 팝: GameplayTag=%s"), *Tag.ToString());
	StaticTagCountContainer.SubtractTagCount(Tag, 1);
}

void UNexusGameplayTagComponent::OnCountMapEvaluated(const TArray<FNexusGameplayTagCount>& OldDynamicTagCountMapItems)
{
	for (const FNexusGameplayTagCount& TagCount : OldDynamicTagCountMapItems)
	{
		if (!DynamicTagCountContainer.Contains(TagCount.Tag))
		{
			OnGameplayTagRemovedDelegate.Broadcast(TagCount.Tag);
		}
	}

	for (const FNexusGameplayTagCount& TagCount : DynamicTagCountContainer.Items)
	{
		if (!OldDynamicTagCountMapItems.Contains(FNexusGameplayTagCount{TagCount.Tag}))
		{
			OnGameplayTagAddedDelegate.Broadcast(TagCount.Tag);
		}
	}
}

void UNexusGameplayTagComponent::OnRep_StaticTagCountContainer()
{
	bIsDirty = true;
}

void UNexusGameplayTagComponent::OnRep_DynamicTagCountContainer(const FNexusGameplayTagCountContainer& OldDynamicTagCountContainer)
{
	OnCountMapEvaluated(OldDynamicTagCountContainer.Items);
}
