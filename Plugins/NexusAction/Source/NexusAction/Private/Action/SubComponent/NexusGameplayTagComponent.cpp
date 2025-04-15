// Fill out your copyright notice in the Description page of Project Settings.


#include "Action/SubComponent/NexusGameplayTagComponent.h"

#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UNexusGameplayTagComponent::UNexusGameplayTagComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UNexusGameplayTagComponent::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer.Reset();
	for (const auto& [Tag,Count] : DynamicTagCountMap)
	{
		if (Count > 0)
		{
			TagContainer.AddTag(Tag);
		}
	}
}

void UNexusGameplayTagComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UNexusGameplayTagComponent, TagCountMap, COND_None, REPNOTIFY_Always);
}

void UNexusGameplayTagComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bIsTagCountMapDirty)
	{
		bIsTagCountMapDirty = false;
		TMap<FGameplayTag, int32> OldDynamicTagCountMap = DynamicTagCountMap;
		DynamicTagCountMap.Empty();
		for (const FNexusGameplayTagCount& TagCount : TagCountMap)
		{
			DynamicTagCountMap.Add(TagCount.Tag) = TagCount.Count;
		}

		for (const auto& [Tag, Count] : TagCountDeltas)
		{
			DynamicTagCountMap.FindOrAdd(Tag) += Count;
		}

		for (const auto& [OldTag, OldCount] : OldDynamicTagCountMap)
		{
			if (!DynamicTagCountMap.Contains(OldTag))
			{
				OnGameplayTagRemovedDelegate.Broadcast(OldTag);
			}
		}

		for (const auto& [NewTag, NewCount] : DynamicTagCountMap)
		{
			if (!OldDynamicTagCountMap.Contains(NewTag))
			{
				OnGameplayTagAddedDelegate.Broadcast(NewTag);
			}
		}
	}
}

void UNexusGameplayTagComponent::PushDynamicTag(const FGameplayTag& Tag)
{
	check(Tag.IsValid());
	bIsTagCountMapDirty = true;

	if (GetOwner()->HasAuthority())
	{
		int32 Index = TagCountMap.Find(Tag);
		if (Index == INDEX_NONE)
		{
			TagCountMap.Add({Tag, 1});
			return;
		}
		if (++TagCountMap[Index].Count == 0)
		{
			TagCountMap.Remove(Tag);
		}
	}
	else
	{
		if (!TagCountDeltas.Contains(Tag))
		{
			TagCountDeltas.Add(Tag, 1);
			return;
		}
		if (++TagCountDeltas[Tag] == 0)
		{
			TagCountDeltas.Remove(Tag);
		}
	}
}

void UNexusGameplayTagComponent::PopDynamicTag(const FGameplayTag& Tag)
{
	check(Tag.IsValid());
	bIsTagCountMapDirty = true;
	if (GetOwner()->HasAuthority())
	{
		int32 Index = TagCountMap.Find(Tag);
		if (Index == INDEX_NONE)
		{
			TagCountMap.Add({Tag, -1});
			return;
		}
		if (--TagCountMap[Index].Count == 0)
		{
			TagCountMap.Remove(Tag);
		}
	}
	else
	{
		if (!TagCountDeltas.Contains(Tag))
		{
			TagCountDeltas.Add(Tag, -1);
			return;
		}
		if (--TagCountDeltas[Tag] == 0)
		{
			TagCountDeltas.Remove(Tag);
		}
	}
}

void UNexusGameplayTagComponent::OnRep_TagCountMap()
{
	bIsTagCountMapDirty = true;
}
