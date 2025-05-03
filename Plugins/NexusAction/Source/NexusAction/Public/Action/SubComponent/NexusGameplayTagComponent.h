// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "NexusAgentBoundComponent.h"
#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "NexusGameplayTagComponent.generated.h"

struct FNexusGameplayTagCountContainer;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNexusGameplayTagAddedSignature, const FGameplayTag&, Tag);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNexusGameplayTagRemovedSignature, const FGameplayTag&, Tag);

USTRUCT()
struct NEXUSACTION_API FNexusGameplayTagCount : public FFastArraySerializerItem
{
	GENERATED_BODY()


	FNexusGameplayTagCount() = default;

	explicit FNexusGameplayTagCount(const FGameplayTag& InTag)
		: Tag(InTag)
	{
	}

	explicit FNexusGameplayTagCount(const FGameplayTag& InTag, int32 InCount)
		: Tag(InTag)
		  , Count(InCount)
	{
	}

	bool operator==(const FNexusGameplayTagCount& Other) const
	{
		return Tag == Other.Tag;
	}

	bool operator!=(const FNexusGameplayTagCount& Other) const
	{
		return !(*this == Other);
	}

	//~ Begin FFastArraySerializerItem Interface.
	// void PostReplicatedAdd(const FNexusGameplayTagCountContainer& InArray);
	// void PreReplicatedRemove(const FNexusGameplayTagCountContainer& InArray);
	// void PostReplicatedChange(const FNexusGameplayTagCountContainer& InArray);
	//~ End FFastArraySerializerItem Interface.


	FString ToString() const { return FString::Printf(TEXT("TagCount={Tag=%s, Count=%d}"), *Tag.ToString(), Count); }

	UPROPERTY()
	FGameplayTag Tag;
	UPROPERTY()
	int32 Count = 0;
};


USTRUCT()
struct NEXUSACTION_API FNexusGameplayTagCountContainer : public FFastArraySerializer
{
	GENERATED_BODY()

	void Init(bool bInHasAuthority)
	{
		bHasAuthority = bInHasAuthority;
	}

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FNexusGameplayTagCount, FNexusGameplayTagCountContainer>(Items, DeltaParms, *this);
	}

	void AddTagCount(const FGameplayTag& InTag, int32 InCount);
	void SubtractTagCount(const FGameplayTag& InTag, int32 InCount);
	void SetTagCount(const FGameplayTag& InTag, int32 InCount);
	int32 GetTagCount(const FGameplayTag& InTag) const;
	void RemoveAllTagCounts();

	bool Contains(const FGameplayTag& InTag) const;


	UPROPERTY()
	TArray<FNexusGameplayTagCount> Items;
	bool bHasAuthority = false;
};

template <>
struct TStructOpsTypeTraits<FNexusGameplayTagCountContainer> : public TStructOpsTypeTraitsBase2<FNexusGameplayTagCountContainer>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NEXUSACTION_API UNexusGameplayTagComponent : public UNexusAgentBoundComponent, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	UNexusGameplayTagComponent();
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EvaluateTagCounts();
	

	void PushDynamicTag(const FGameplayTag& Tag);
	void PopDynamicTag(const FGameplayTag& Tag);
	void PushStaticTag(const FGameplayTag& Tag);
	void PopStaticTag(const FGameplayTag& Tag);
	

	const FNexusGameplayTagCountContainer& GetDynamicTagCountContainer() const { return DynamicTagCountContainer; }

private:
	void OnCountMapEvaluated(const TArray<FNexusGameplayTagCount>& OldDynamicTagCountMapItems);
	UFUNCTION()
	void OnRep_StaticTagCountContainer();
	UFUNCTION()
	void OnRep_DynamicTagCountContainer(const FNexusGameplayTagCountContainer& OldDynamicTagCountContainer);

	
public:
	UPROPERTY(BlueprintAssignable)
	FOnNexusGameplayTagAddedSignature OnGameplayTagAddedDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnNexusGameplayTagRemovedSignature OnGameplayTagRemovedDelegate;

private:
	UPROPERTY(ReplicatedUsing = OnRep_StaticTagCountContainer)
	FNexusGameplayTagCountContainer StaticTagCountContainer;
	UPROPERTY(ReplicatedUsing = OnRep_DynamicTagCountContainer)
	FNexusGameplayTagCountContainer DynamicTagCountContainer;

	TMap<FGameplayTag, int32> DynamicTagCountDeltas;

	bool bIsDirty = false;
};
