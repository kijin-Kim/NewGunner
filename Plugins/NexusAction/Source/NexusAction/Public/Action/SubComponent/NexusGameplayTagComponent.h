// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "NexusAgentBoundComponent.h"
#include "Components/ActorComponent.h"
#include "NexusGameplayTagComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNexusGameplayTagAddedSignature, const FGameplayTag&, Tag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNexusGameplayTagRemovedSignature, const FGameplayTag&, Tag);

USTRUCT()
struct FNexusGameplayTagCount
{
	GENERATED_BODY()

	FNexusGameplayTagCount() = default;

	FNexusGameplayTagCount(const FGameplayTag& InTag)
		: Tag(InTag)
	{
	}

	FNexusGameplayTagCount(const FGameplayTag& InTag, int32 InCount)
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

	UPROPERTY()
	FGameplayTag Tag;
	UPROPERTY()
	int32 Count = 0;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NEXUSACTION_API UNexusGameplayTagComponent : public UNexusAgentBoundComponent, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	UNexusGameplayTagComponent();
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	void PushDynamicTag(const FGameplayTag& Tag);
	void PopDynamicTag(const FGameplayTag& Tag);

	const TMap<FGameplayTag, int32>& GetDynamicTagCountMap() const { return DynamicTagCountMap; }

private:
	UFUNCTION()
	void OnRep_TagCountMap();

public:
	UPROPERTY(BlueprintAssignable)
	FOnNexusGameplayTagAddedSignature OnGameplayTagAddedDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnNexusGameplayTagRemovedSignature OnGameplayTagRemovedDelegate;

private:
	UPROPERTY(ReplicatedUsing = OnRep_TagCountMap)
	TArray<FNexusGameplayTagCount> TagCountMap;
	TMap<FGameplayTag, int32> TagCountDeltas; // 곱셈, 나눗셈 연산이 없기 때문에 단순히 카운트로 관리
	TMap<FGameplayTag, int32> DynamicTagCountMap;
	bool bIsTagCountMapDirty = false;
	
};
