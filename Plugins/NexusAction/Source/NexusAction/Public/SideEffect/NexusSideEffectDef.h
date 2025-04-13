// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NexusSideEffectDefHandle.h"
#include "Prediction/NexusPrediction.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "NexusSideEffectDef.generated.h"

struct FNexusSideEffectDef;
class UNexusSideEffect;




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
	void PostReplicatedChange(const struct FNexusSideEffectDefContainer& InArraySerializer);

	
	FNexusSideEffectDefHandle Handle;
	UPROPERTY()
	TSubclassOf<UNexusSideEffect> SideEffectClass;

	UPROPERTY(NotReplicated, BlueprintReadOnly)
	TObjectPtr<UNexusSideEffect> SideEffectInstance;

	UPROPERTY()
	FNexusPredictionTag PredictionTag;
	
};

USTRUCT()
struct FNexusSideEffectDefContainer : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	void Init(AActor* InOwnerActor);
	void Add(const FNexusSideEffectDef& SideEffectDef, FNexusPredictionTag PredictionTag);
	void Remove(const FNexusSideEffectDefHandle& SideEffectDefHandle);
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);

	void OnAdded(FNexusSideEffectDef& SideEffectDef) const;
	void OnRemoved(const FNexusSideEffectDef& SideEffectDef) const;

	void Tick(float DeltaTime);
	FNexusSideEffectDef* FindSideEffectDefByHandle(FNexusSideEffectDefHandle Handle);


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
