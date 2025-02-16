// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NexusCue.h"
#include "UObject/Object.h"
#include "GunnerTargetData_Hit.generated.h"

USTRUCT()
struct FGunnerTargetData_Hit : public FNexusTargetDataBase
{
	GENERATED_BODY()

public:
	virtual UScriptStruct* GetStructType() const override { return FGunnerTargetData_Hit::StaticStruct(); }
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

public:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> AgentActor;
	UPROPERTY(BlueprintReadOnly)
	TArray<FHitResult> LocalHitResult;
};


template <>
struct TStructOpsTypeTraits<FGunnerTargetData_Hit> : public TStructOpsTypeTraitsBase2<FGunnerTargetData_Hit>
{
	enum
	{
		WithNetSerializer = true
	};
};
