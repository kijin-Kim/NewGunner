// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GunnerActionSideEffectDefinitionHandle.h"
#include "AsyncAction/GunnerActionNetPrediction.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "GunnerActionSideEffectDefinition.generated.h"

struct FGunnerActionSideEffectDefinition;
struct FGunnerActionPropertySideEffect;
class UGunnerActionSideEffect;


DECLARE_DELEGATE_TwoParams(FOnSideEffectDefinitionAddedSignature, const FGunnerActionSideEffectDefinition& /*SideEffectDefinition*/, FGunnerActionNetPredictionHandle /*PredictionHandle*/);
DECLARE_DELEGATE_OneParam(FOnSideEffectDefinitionRemovedSignature, FGunnerActionSideEffectDefinitionHandle /*SideEffectDefinitionHandle*/);



/**
 * 
 */
USTRUCT(BlueprintType)
struct FGunnerActionSideEffectDefinition
{
	GENERATED_BODY()

	FGunnerActionSideEffectDefinition();
	FGunnerActionSideEffectDefinition(TSubclassOf<UGunnerActionSideEffect> InActionClass);
	bool operator==(const FGunnerActionSideEffectDefinition& Other) const;
	bool operator!=(const FGunnerActionSideEffectDefinition& Other) const;

	UPROPERTY()
	FGunnerActionSideEffectDefinitionHandle Handle;
	UPROPERTY()
	TSubclassOf<UGunnerActionSideEffect> SideEffectClass;

	UPROPERTY()
	TObjectPtr<UGunnerActionSideEffect> SideEffectCDO;

	UPROPERTY(BlueprintReadWrite)
	TMap<FGameplayTag, float> OutsideSourceValues;
};


struct FGunnerActionSideEffectDefinitionArray;

USTRUCT()
struct FGunnerActionSideEffectDefinitionItem : public FFastArraySerializerItem
{
	GENERATED_USTRUCT_BODY()

	void PostReplicatedAdd(const struct FGunnerActionSideEffectDefinitionArray& InArraySerializer);
	void PreReplicatedRemove(const struct FGunnerActionSideEffectDefinitionArray& InArraySerializer);
	void PostReplicatedChange(const struct FGunnerActionSideEffectDefinitionArray& InArraySerializer);
	

	UPROPERTY()
	FGunnerActionSideEffectDefinition SideEffectDefinition;
	UPROPERTY()
	FGunnerActionNetPredictionHandle PredictionHandle;
};

USTRUCT()
struct FGunnerActionSideEffectDefinitionArray : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	void Add(const FGunnerActionSideEffectDefinition& SideEffectDefinition, FGunnerActionNetPredictionHandle PredictionHandle, bool bHasAuthority);
	void OnAdded(const FGunnerActionSideEffectDefinition& SideEffectDefinition, FGunnerActionNetPredictionHandle PredictionHandle) const;

	void Remove(const FGunnerActionSideEffectDefinitionHandle& SideEffectDefinitionHandle);
	void OnRemoved(const FGunnerActionSideEffectDefinitionHandle& SideEffectDefinitionHandle) const;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);

	UPROPERTY()
	TArray<FGunnerActionSideEffectDefinitionItem> Items;

	FOnSideEffectDefinitionAddedSignature OnSideEffectDefinitionAddedDelegate;
	FOnSideEffectDefinitionRemovedSignature OnSideEffectDefinitionRemovedDelegate;
};

template <>
struct TStructOpsTypeTraits<FGunnerActionSideEffectDefinitionArray> : public TStructOpsTypeTraitsBase2<FGunnerActionSideEffectDefinitionArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
