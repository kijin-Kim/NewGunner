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

DECLARE_DELEGATE_OneParam(FOnNexusCueAddedSignature, FNexusLoopingCue& /*LoopingCue*/);
DECLARE_DELEGATE_OneParam(FOnNexusCueRemovedSignature, FNexusLoopingCue& /*LoopingCue*/);
DECLARE_DELEGATE(FOnNexusCueDurationExpiredSignature);


UENUM()
enum class ENexusCueType : uint8
{
	Burst,
	Looping
};

USTRUCT(BlueprintType)
struct FNexusLoopingCueHandle
{
	GENERATED_BODY()

	FNexusLoopingCueHandle()
		: Handle(INDEX_NONE)
	{
	}

	void GenerateNewHandle();
	bool IsValid() const { return Handle != INDEX_NONE; }
	bool operator==(const FNexusLoopingCueHandle& Other) const = default;
	FString ToString() const { return FString::Printf(TEXT("LoopingCueHandle={Handle=%d}"), Handle); }

	friend uint32 GetTypeHash(const FNexusLoopingCueHandle& CueHandle) { return GetTypeHash(CueHandle.Handle); }

private:
	UPROPERTY()
	int32 Handle;
};


USTRUCT()
struct FNexusLoopingCue : public FFastArraySerializerItem
{
	GENERATED_USTRUCT_BODY()

	FNexusLoopingCue();

	bool operator==(const FNexusLoopingCue& Other) const;
	bool operator!=(const FNexusLoopingCue& Other) const;

	void PreReplicatedRemove(const struct FNexusLoopingCueContainer& InArraySerializer);
	void PostReplicatedAdd(const struct FNexusLoopingCueContainer& InArraySerializer);
	void PostReplicatedChange(const struct FNexusLoopingCueContainer& InArraySerializer);

	UPROPERTY()
	TSubclassOf<ANexusCue> CueClass;
	UPROPERTY()
	FNexusTargetDataHandle TargetDataHandle;
	// 각 로컬에서만 유효한 핸들
	UPROPERTY()
	FNexusLoopingCueHandle Handle;

	UPROPERTY(NotReplicated)
	TObjectPtr<ANexusCue> CueActor;
};


USTRUCT()
struct FNexusLoopingCueContainer : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);

	FNexusLoopingCueHandle AddLoopingCue(const FNexusLoopingCue& InLoopingCue, bool bHasAuthority);
	void RemoveLoopingCue(FNexusLoopingCueHandle Handle, bool bHasAuthority);
	void RemoveAllLoopingCues();

	void OnAdded(FNexusLoopingCue& LoopingCue) const;
	void OnRemoved(FNexusLoopingCue& LoopingCue) const;

	FNexusLoopingCue* FindLoopingCueByHandle(const FNexusLoopingCueHandle& InHandle);

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
	void CallOnBecomeRelevant(const FNexusTargetDataHandle& InTargetDataHandle);
	void CallOnCeaseRelevant();

	UFUNCTION(BlueprintCallable)
	void EndCue() const;

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

public:
	FOnNexusCueDurationExpiredSignature OnDurationExpiredDelegate;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	ENexusCueType CueType = ENexusCueType::Burst;
	// 루핑 타입의 큐의 지속시간. 0보다 작을 경우 무한 지속으로 간주
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true, EditCondition = "DurationType == ENexusCueDurationType::Duration"))
	float Duration = 0.0f;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	FNexusTargetDataHandle TargetDataHandle;

	FTimerHandle DurationExpiredTimerHandle;
};
