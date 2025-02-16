// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NexusSideEffectDefHandle.h"
#include "NexusPrediction.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "NexusSideEffectDef.generated.h"

struct FNexusSideEffectDef;
class UNexusSideEffect;


DECLARE_DELEGATE_TwoParams(FOnSideEffectDefinitionAddedSignature, const FNexusSideEffectDef& /*SideEffectDefinition*/, FNexusPredictionTag /*PredictionHandle*/);
DECLARE_DELEGATE_OneParam(FOnSideEffectDefinitionRemovedSignature, FNexusSideEffectDefHandle /*SideEffectDefinitionHandle*/);


/**
 * 
 */
USTRUCT(BlueprintType)
struct FNexusSideEffectDef : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FNexusSideEffectDef();
	FNexusSideEffectDef(TSubclassOf<UNexusSideEffect> InActionClass);
	bool operator==(const FNexusSideEffectDef& Other) const;
	bool operator!=(const FNexusSideEffectDef& Other) const;

	void PostReplicatedAdd(const struct FNexusSideEffectDefContainer& InArraySerializer);
	void PreReplicatedRemove(const struct FNexusSideEffectDefContainer& InArraySerializer);

	
	FNexusSideEffectDefHandle Handle;
	UPROPERTY()
	TSubclassOf<UNexusSideEffect> SideEffectClass;

	UPROPERTY(NotReplicated, BlueprintReadOnly)
	TObjectPtr<UNexusSideEffect> SideEffectInstance;

	UPROPERTY()
	FNexusPredictionTag PredictionHandle;
};

USTRUCT()
struct FNexusSideEffectDefContainer : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	void Init(AActor* InOwnerActor);
	void Add(const FNexusSideEffectDef& SideEffectDefinition, FNexusPredictionTag PredictionHandle);
	void Remove(const FNexusSideEffectDefHandle& SideEffectDefinitionHandle);
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);

	void OnAdded(FNexusSideEffectDef& SideEffectDefinition) const;
	void OnRemoved(const FNexusSideEffectDef& SideEffectDefinition) const;

	void Tick(float DeltaTime);


	UPROPERTY()
	TArray<FNexusSideEffectDef> Items;

private:
	AActor* OwnerActor;
	bool bHasAuthority;
};

template <>
struct TStructOpsTypeTraits<FNexusSideEffectDefContainer> : public TStructOpsTypeTraitsBase2<FNexusSideEffectDefContainer>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
