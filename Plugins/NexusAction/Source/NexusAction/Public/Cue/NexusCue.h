// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "UObject/Object.h"
#include "NexusCue.generated.h"

struct FNexusCueParameters;
struct FNexusAgentInfo;
struct FNexusLoopingCue;
class ANexusCue;

DECLARE_DELEGATE_OneParam(FOnNexusCueAddedSignature, const FNexusLoopingCue& /*LoopingCue*/);
DECLARE_DELEGATE_OneParam(FOnNexusCueRemovedSignature, const FNexusLoopingCue& /*LoopingCue*/);


USTRUCT(BlueprintType)
struct NEXUSACTION_API FNexusCueParameters
{
	GENERATED_BODY()


	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

public:
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AActor> Causer;
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AActor> Target;

	UPROPERTY(BlueprintReadWrite)
	FVector_NetQuantize Location = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	FVector_NetQuantizeNormal Normal = FVector::ForwardVector;

	UPROPERTY(BlueprintReadWrite)
	TArray<FHitResult> HitResults;
};

template <>
struct TStructOpsTypeTraits<FNexusCueParameters> : public TStructOpsTypeTraitsBase2<FNexusCueParameters>
{
	enum
	{
		WithNetSerializer = true
	};
};


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

	explicit FNexusLoopingCue(TSubclassOf<ANexusCue> InCueClass, const FNexusCueParameters& InCueParameters)
		: CueClass(InCueClass),
		  CueParameters(InCueParameters)
	{
		Handle.GenerateNewHandle();
	}

	bool operator==(const FNexusLoopingCue& Other) const;
	bool operator!=(const FNexusLoopingCue& Other) const;


	void PreReplicatedRemove(const struct FNexusLoopingCueContainer& InArraySerializer);


	FNexusLoopingCueHandle GetHandle() const { return Handle; }

	FString ToString() const;

public:
	UPROPERTY()
	TSubclassOf<ANexusCue> CueClass;
	UPROPERTY()
	FNexusCueParameters CueParameters;
	bool bIsAdded = false;

private:
	// 각 로컬에서만 유효한 핸들
	FNexusLoopingCueHandle Handle;
};


USTRUCT()
struct FNexusLoopingCueContainer : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()


	
	void Init(TWeakObjectPtr<AActor> InOwnerActor, TWeakObjectPtr<AActor> InAgentActor);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);

	FNexusLoopingCueHandle AddLoopingCue(const FNexusLoopingCue& InLoopingCue, bool bHasAuthority);
	void RemoveLoopingCue(FNexusLoopingCueHandle Handle, bool bHasAuthority);
	void RemoveAllLoopingCues();
	
	void PostReplicatedReceive(const FFastArraySerializer::FPostReplicatedReceiveParameters& Parameters);


	FNexusLoopingCue* FindLoopingCueByHandle(const FNexusLoopingCueHandle& InHandle);
	FNexusLoopingCue* FindLoopingCueByClass(TSubclassOf<ANexusCue> InCueClass);

	void OnAdded(FNexusLoopingCue& LoopingCue) const;
	void OnRemoved(const FNexusLoopingCue& LoopingCue) const;
	void FlushPendingAdds();

public:


	UPROPERTY()
	TArray<FNexusLoopingCue> Items;

	FOnNexusCueAddedSignature OnCueAddedDelegate;
	FOnNexusCueRemovedSignature OnCueRemovedDelegate;

	bool bInitialized = false;
	TWeakObjectPtr<AActor> OwnerActor;
	TWeakObjectPtr<AActor> AgentActor;
};

template <>
struct TStructOpsTypeTraits<FNexusLoopingCueContainer> : public TStructOpsTypeTraitsBase2<FNexusLoopingCueContainer>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};


UENUM()
enum class ENexusCueState : uint8
{
	None,
	Triggered,
	BecomeRelevant
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
	void CallOnTriggered(const FNexusCueParameters& InCueParameters, AActor* AgentActor, AActor* OwnerActor);
	void CallOnBecomeRelevant(const FNexusCueParameters& InCueParameters, AActor* AgentActor, AActor* OwnerActor);
	void CallOnCeaseRelevant(AActor* AgentActor, AActor* OwnerActor);


	float GetDuration() const { return Duration; }
	ENexusCueType GetCueType() const { return CueType; }
	ENexusCueState GetCueState() const { return CueState; }


protected:
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnTriggered"))
	void BP_OnTriggered(const FNexusCueParameters& InCueParameters, AActor* AgentActor, AActor* OwnerActor);
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnBecomeRelevant"))
	void BP_OnBecomeRelevant(const FNexusCueParameters& InCueParameters, AActor* AgentActor, AActor* OwnerActor);
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnCeaseRelevant"))
	void BP_OnCeaseRelevant(AActor* AgentActor, AActor* OwnerActor);

	virtual void OnTriggered(const FNexusCueParameters& InCueParameters, AActor* AgentActor, AActor* OwnerActor)
	{
	}

	virtual void OnBecomeRelevant(const FNexusCueParameters& InCueParameters, AActor* AgentActor, AActor* OwnerActor)
	{
	}

	virtual void OnCeaseRelevant(AActor* AgentActor, AActor* OwnerActor)
	{
	}




private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	ENexusCueType CueType = ENexusCueType::Burst;
	// 루핑 타입의 큐의 지속시간. 0보다 작을 경우 무한 지속으로 간주
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true, EditCondition = "CueType == ENexusCueType::Looping"))
	float Duration = 0.0f;

	ENexusCueState CueState = ENexusCueState::None;
};
