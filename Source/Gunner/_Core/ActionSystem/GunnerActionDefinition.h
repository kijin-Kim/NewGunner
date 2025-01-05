// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerActionDefinitionHandle.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "GunnerActionDefinition.generated.h"

/**
 * 
 */

struct FGunnerActionDefinition;
class UGunnerAction;

DECLARE_DELEGATE_OneParam(FOnActionDefinitionAddedSignature, FGunnerActionDefinition& /*ActionDefinition*/);
DECLARE_DELEGATE_OneParam(FOnActionDefinitionRemovedSignature, FGunnerActionDefinition& /*ActionDefinition*/);


USTRUCT()
struct FGunnerActionDefinition : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FGunnerActionDefinition();
	FGunnerActionDefinition(UObject* InSourceObject, TSubclassOf<UGunnerAction> InActionClass);
	bool operator==(const FGunnerActionDefinition& Other) const;
	bool operator!=(const FGunnerActionDefinition& Other) const;

	void PostReplicatedAdd(const struct FGunnerActionDefinitionArray& InArraySerializer);
	void PreReplicatedRemove(const struct FGunnerActionDefinitionArray& InArraySerializer);
	void PostReplicatedChange(const struct FGunnerActionDefinitionArray& InArraySerializer);

	UPROPERTY()
	FGunnerActionDefinitionHandle Handle;
	UPROPERTY()
	TSubclassOf<UGunnerAction> ActionClass;
	UPROPERTY(NotReplicated)
	TObjectPtr<UGunnerAction> ActionInstance;
};

USTRUCT()
struct FGunnerActionDefinitionArray : public FFastArraySerializer
{
	GENERATED_BODY()

	FGunnerActionDefinitionArray();

	void AuthAdd(const FGunnerActionDefinition& ActionDefinition);
	void AuthRemove(const FGunnerActionDefinitionHandle& Handle);
	void AuthRemoveAll();
	FGunnerActionDefinition* FindActionDefinitionByHandle(const FGunnerActionDefinitionHandle& Handle);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);
	void OnAdded(FGunnerActionDefinition& ActionDefinition) const;
	void OnRemoved(FGunnerActionDefinition& ActionDefinition) const;
	

	UPROPERTY()
	TArray<FGunnerActionDefinition> Items;
	FOnActionDefinitionAddedSignature OnActionDefinitionAddedDelegate;
	FOnActionDefinitionRemovedSignature OnActionDefinitionRemovedDelegate;
};


template <>
struct TStructOpsTypeTraits<FGunnerActionDefinitionArray> : public TStructOpsTypeTraitsBase2<FGunnerActionDefinitionArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
