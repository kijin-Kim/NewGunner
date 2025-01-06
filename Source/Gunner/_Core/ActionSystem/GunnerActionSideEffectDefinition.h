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
struct FGunnerActionSideEffectDefinition : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FGunnerActionSideEffectDefinition();
	FGunnerActionSideEffectDefinition(TSubclassOf<UGunnerActionSideEffect> InActionClass);
	bool operator==(const FGunnerActionSideEffectDefinition& Other) const;
	bool operator!=(const FGunnerActionSideEffectDefinition& Other) const;

	void PostReplicatedAdd(const struct FGunnerActionSideEffectDefinitionArray& InArraySerializer);
	void PreReplicatedRemove(const struct FGunnerActionSideEffectDefinitionArray& InArraySerializer);
	void PostReplicatedChange(const struct FGunnerActionSideEffectDefinitionArray& InArraySerializer);
	
	UPROPERTY()
	FGunnerActionSideEffectDefinitionHandle Handle;
	UPROPERTY()
	TSubclassOf<UGunnerActionSideEffect> SideEffectClass;

	UPROPERTY(NotReplicated, BlueprintReadOnly)
	TObjectPtr<UGunnerActionSideEffect> SideEffectInstance;

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

	void Tick(float DeltaTime);


	UPROPERTY()
	TArray<FGunnerActionSideEffectDefinition> Items;
};

template <>
struct TStructOpsTypeTraits<FGunnerActionSideEffectDefinitionArray> : public TStructOpsTypeTraitsBase2<FGunnerActionSideEffectDefinitionArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
