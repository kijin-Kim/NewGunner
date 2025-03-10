// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TargetData/NexusTargetData.h"
#include "UObject/Object.h"
#include "GunnerTargetData_Actor.generated.h"

/**
 * 
 */
USTRUCT()
struct FGunnerTargetData_Actor : public FNexusTargetDataBase
{
	GENERATED_BODY()

public:
	virtual UScriptStruct* GetStructType() const override { return FGunnerTargetData_Actor::StaticStruct(); }
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

public:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> Actor;
};


template <>
struct TStructOpsTypeTraits<FGunnerTargetData_Actor> : public TStructOpsTypeTraitsBase2<FGunnerTargetData_Actor>
{
	enum
	{
		WithNetSerializer = true
	};
};
