// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NexusActionDefHandle.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "NexusActionDef.generated.h"

/**
 * 
 */

struct FNexusActionDef;
class UNexusAction;

DECLARE_DELEGATE_OneParam(FOnActionDefinitionAddedSignature, FNexusActionDef& /*ActionDefinition*/);
DECLARE_DELEGATE_OneParam(FOnActionDefinitionRemovedSignature, FNexusActionDef& /*ActionDefinition*/);


struct FNexusActionDefContainer;

USTRUCT()
struct NEXUSACTION_API FNexusActionDef : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FNexusActionDef();
	FNexusActionDef(UObject* InSourceObject, TSubclassOf<UNexusAction> InActionClass);
	bool operator==(const FNexusActionDef& Other) const;
	bool operator!=(const FNexusActionDef& Other) const;

	void PostReplicatedAdd(const FNexusActionDefContainer& InArraySerializer);
	void PreReplicatedRemove(const FNexusActionDefContainer& InArraySerializer);

	UPROPERTY()
	FNexusActionDefHandle Handle;
	UPROPERTY()
	TSubclassOf<UNexusAction> ActionClass;
	UPROPERTY(NotReplicated)
	TObjectPtr<UNexusAction> ActionInstance;
};

USTRUCT()
struct NEXUSACTION_API FNexusActionDefContainer : public FFastArraySerializer
{
	GENERATED_BODY()


	void AuthAdd(const FNexusActionDef& ActionDefinition);
	void AuthRemove(const FNexusActionDefHandle& Handle);
	void AuthRemoveAll();
	FNexusActionDef* FindActionDefinitionByHandle(const FNexusActionDefHandle& Handle);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);
	void OnAdded(FNexusActionDef& ActionDefinition) const;
	void OnRemoved(FNexusActionDef& ActionDefinition) const;
	

	UPROPERTY()
	TArray<FNexusActionDef> Items;
	FOnActionDefinitionAddedSignature OnActionDefinitionAddedDelegate;
	FOnActionDefinitionRemovedSignature OnActionDefinitionRemovedDelegate;
};


template <>
struct TStructOpsTypeTraits<FNexusActionDefContainer> : public TStructOpsTypeTraitsBase2<FNexusActionDefContainer>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
