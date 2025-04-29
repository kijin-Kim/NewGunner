// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NexusProperty.h"
#include "NexusSideEffectInstanceHandle.h"
#include "Prediction/NexusPrediction.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "NexusSideEffectInstance.generated.h"

class UNexusGameplayTagComponent;
class UNexusPropertyComponent;
class UNexusActionComponent;
class UNexusSideEffectComponent;
struct FNexusGameplayTagMod;
struct FNexusPropertyMod;
struct FNexusSideEffectInstance;
class UNexusSideEffect;


/**
 * 사이드이펙트 실행 관련 동적, 정적(UNexusSideEffect) 데이터를 가지고 있는 구조체
 */

USTRUCT()
struct FNexusInjectedValuePair
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag Tag;
	UPROPERTY()
	float Value = 0.0f;
};

USTRUCT()
struct NEXUSACTION_API FNexusSideEffectInstanceDef
{
	GENERATED_BODY()

public:
	FNexusSideEffectInstanceDef();
	explicit FNexusSideEffectInstanceDef(TSubclassOf<UNexusSideEffect> InSideEffectClass);
	FString ToString() const;

	// 정적 사이드이펙트 데이터
	UPROPERTY()
	TObjectPtr<const UNexusSideEffect> SideEffectAsset;
	UPROPERTY()
	TArray<FNexusInjectedValuePair> InjectedValues;
	UPROPERTY()
	TArray<FNexusGameplayTagMod> DynamicTagModifiers;

	TArray<FNexusPropertyOperationHandle> AppliedOperationHandles;
};

USTRUCT(BlueprintType)
struct FNexusSideEffectInstanceDefHandle
{
	GENERATED_BODY()

public:
	FNexusSideEffectInstanceDefHandle() = default;

	FNexusSideEffectInstanceDefHandle(const FNexusSideEffectInstanceDefHandle& Other) : Data(Other.Data)
	{
	}

	FNexusSideEffectInstanceDefHandle& operator=(const FNexusSideEffectInstanceDefHandle& Other)
	{
		Data = Other.Data;
		return *this;
	}

	FNexusSideEffectInstanceDefHandle(FNexusSideEffectInstanceDefHandle&& Other)
	{
		Data = MoveTemp(Other.Data);
	};

	FNexusSideEffectInstanceDefHandle& operator=(FNexusSideEffectInstanceDefHandle&& Other)
	{
		Data = MoveTemp(Other.Data);
		return *this;
	}

	void Reset() { Data.Reset(); }
	bool IsValid() const { return Data.IsValid(); }
	void SetData(TSharedPtr<FNexusSideEffectInstanceDef> InData) { Data = InData; }
	TSharedPtr<FNexusSideEffectInstanceDef> GetData() const { return Data; }

private:
	TSharedPtr<FNexusSideEffectInstanceDef> Data;
};


/**
 *  사이드이펙트의 실행 인스턴스. 모디파이어를 적용하고 자신의 지속시간을 관리
 */
USTRUCT()
struct FNexusSideEffectInstance : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FNexusSideEffectInstance();
	explicit FNexusSideEffectInstance(const FNexusSideEffectInstanceDef& InDef);

	bool operator==(const FNexusSideEffectInstance& Other) const;
	bool operator!=(const FNexusSideEffectInstance& Other) const;
	void InitializeSideEffectInstance(UNexusPropertyComponent* InPropertyComponent, UNexusGameplayTagComponent* InGameplayTagComponent, bool bInHasAuthority);

	void PostReplicatedAdd(const struct FNexusSideEffectInstanceContainer& InArraySerializer);
	void PreReplicatedRemove(const struct FNexusSideEffectInstanceContainer& InArraySerializer);
	void PostReplicatedChange(const struct FNexusSideEffectInstanceContainer& InArraySerializer);


	void OnApplied();
	void OnTick(float DeltaTime);
	void OnRemoved() const;
	void ApplyPropertyModifier(const FNexusPropertyMod& Modifier);
	void ApplyTagModifier(const FNexusGameplayTagMod& Modifier);
	void ApplyAllModifiers();
	bool IsExpired() const;

	FString ToString() const
	{
		return FString::Printf(TEXT("SideEffectInstance={%s, Def=%s, RemainingDuration=%.2f, ElapsedTime=%.2f, Interval=%.2f, AppliedCount=%d}"),
		                       *Handle.ToString(), *Def.ToString(), RemainingDuration, ElapsedTime, Interval, AppliedCount);
	}

	FNexusSideEffectInstanceHandle Handle;

	UPROPERTY()
	FNexusSideEffectInstanceDef Def;

	UPROPERTY()
	float RemainingDuration;
	UPROPERTY()
	float ElapsedTime;
	UPROPERTY()
	float Interval;
	UPROPERTY()
	int32 AppliedCount;


	UPROPERTY()
	FNexusPredictionTag PredictionTag;


	TWeakObjectPtr<UNexusGameplayTagComponent> GameplayTagComponent;
	TWeakObjectPtr<UNexusPropertyComponent> PropertyComponent;
	bool bHasAuthority = false;
};

USTRUCT()
struct FNexusSideEffectInstanceContainer : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	void Init(UNexusPropertyComponent* InPropertyComponent, UNexusGameplayTagComponent* InGameplayTagComponent, bool bInHasAuthority);
	FNexusSideEffectInstanceHandle ApplySideEffectByDef(const FNexusSideEffectInstanceDef& SideEffectInstanceDef);
	void RemoveSideEffectInstance(const FNexusSideEffectInstanceHandle& SideEffectInstanceHandle);
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);

	void OnSideEffectInstanceAdded(FNexusSideEffectInstance& SideEffectInstance) const;
	void OnSideEffectInstanceRemoved(const FNexusSideEffectInstance& SideEffectInstance) const;
	void Tick(float DeltaTime);
	
	void IncreaseSideEffectContainerLock();
	void DecreaseSideEffectContainerLock();

private:
	FNexusSideEffectInstanceHandle InternalApplySideEffectByInstance(const FNexusSideEffectInstance& SideEffectInstance);
	int32 RemoveSideEffectInstanceByPredicate(const TFunction<bool(const FNexusSideEffectInstance&)>& Predicate);

public:
	UPROPERTY()
	TArray<FNexusSideEffectInstance> SideEffectInstances;
	TWeakObjectPtr<UNexusPropertyComponent> PropertyComponent;
	TWeakObjectPtr<UNexusGameplayTagComponent> GameplayTagComponent;
	
private:

	bool TEMP_LOOPING = false;
	
	bool bHasAuthority = false;
	int32 ScopeLockCount = 0;
	TArray<FNexusSideEffectInstance> PendingAdds;
	TArray<FNexusSideEffectInstanceHandle> PendingRemoves;

};

template <>
struct TStructOpsTypeTraits<FNexusSideEffectInstanceContainer> : public TStructOpsTypeTraitsBase2<FNexusSideEffectInstanceContainer>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};


struct FNexusSideEffectContainerLock
{
	FNexusSideEffectContainerLock(FNexusSideEffectInstanceContainer& InSideEffectContainer);
	~FNexusSideEffectContainerLock();

	FNexusSideEffectInstanceContainer& SideEffectContainer;
};

#define EFFECT_CONTAINER_SCOPE_LOCK() FNexusSideEffectContainerLock SideEffectContScopeLock(*this)
