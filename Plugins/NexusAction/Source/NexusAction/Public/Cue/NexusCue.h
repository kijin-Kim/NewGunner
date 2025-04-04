// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NexusLog.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "TargetData/NexusTargetData.h"
#include "UObject/Object.h"
#include "NexusCue.generated.h"

struct FNexusLoopingCue;
class ANexusCue;

DECLARE_DELEGATE_OneParam(FOnNexusCueAddedSignature, const FNexusLoopingCue& /*LoopingCue*/);
DECLARE_DELEGATE_OneParam(FOnNexusCueRemovedSignature, TSubclassOf<ANexusCue> /*CueClass*/);
DECLARE_DELEGATE(FOnNexusCueDurationExpiredSignature);


UENUM()
enum class ENexusCueType : uint8
{
	Burst,
	Looping
};


USTRUCT()
struct FNexusLoopingCue : public FFastArraySerializerItem
{
	GENERATED_USTRUCT_BODY()

	void PreReplicatedRemove(const struct FNexusLoopingCueContainer& InArraySerializer);
	void PostReplicatedAdd(const struct FNexusLoopingCueContainer& InArraySerializer);
	void PostReplicatedChange(const struct FNexusLoopingCueContainer& InArraySerializer);

	UPROPERTY()
	TSubclassOf<ANexusCue> CueClass;
	UPROPERTY()
	FNexusTargetDataHandle TargetDataHandle;
};

USTRUCT()
struct FNexusLoopingCueContainer : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);

	void AddLoopingCue(const FNexusLoopingCue& InLoopingCue, bool bHasAuthority);
	void RemoveLoopingCue(TSubclassOf<ANexusCue> InCueClass, bool bHasAuthority);

	void OnAdded(const FNexusLoopingCue& LoopingCue) const;
	void OnRemoved(TSubclassOf<ANexusCue> CueClass) const;

	UPROPERTY()
	TArray<FNexusLoopingCue> Items;

	FOnNexusCueAddedSignature OnCueAddedDelegate;
	FOnNexusCueRemovedSignature OnCueRemovedDelegate;
};

template <>
struct TStructOpsTypeTraits<FNexusLoopingCueContainer> : public TStructOpsTypeTraitsBase2<FNexusLoopingCueContainer>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};


/**
 * 
 */
UCLASS(Blueprintable, meta = (ShowWorldContextPin))
class NEXUSACTION_API ANexusCue : public AActor
{
	GENERATED_BODY()

public:
	ANexusCue();
	void CallOnTriggered(const FNexusTargetDataHandle& InTargetDataHandle);
	void CallOnBecomeRelevant();
	void CallOnCeaseRelevant();

	ENexusCueType GetCueType() const { return CueType; }

protected:
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnTriggered"))
	void BP_OnTriggered();
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnBecomeRelevant"))
	void BP_OnBecomeRelevant();
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnCeaseRelevant"))
	void BP_OnCeaseRelevant();

	virtual void OnTriggered(const FNexusTargetDataHandle& InTargetDataHandle);
	virtual void OnBecomeRelevant();
	virtual void OnCeaseRelevant();

private:
	void OnDurationExpired() const;

public:
	FOnNexusCueDurationExpiredSignature OnDurationExpiredDelegate;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	ENexusCueType CueType = ENexusCueType::Burst;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true, EditCondition = "DurationType == ENexusCueDurationType::Duration"))
	float Duration = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true, EditCondition = "DurationType == ENexusCueDurationType::Duration"))
	bool bIsInfiniteDuration = false;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	FNexusTargetDataHandle TargetDataHandle;
	
	FTimerHandle DurationExpiredTimerHandle;
};
