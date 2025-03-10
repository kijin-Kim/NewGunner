// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TargetData/NexusTargetData.h"
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
	float TimeStamp = 0.0f;
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> AgentActor;
	UPROPERTY(BlueprintReadOnly)
	TArray<FHitResult> HitResults;
};


template <>
struct TStructOpsTypeTraits<FGunnerTargetData_Hit> : public TStructOpsTypeTraitsBase2<FGunnerTargetData_Hit>
{
	enum
	{
		WithNetSerializer = true
	};
};
