// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TargetData/NexusTargetData.h"
#include "UObject/Object.h"
#include "GunnerTargetData_SoundBase.generated.h"

/**
 * 
 */
USTRUCT()
struct FGunnerTargetData_SoundBase : public FNexusTargetDataBase
{
	GENERATED_BODY()

public:
	virtual UScriptStruct* GetStructType() const override { return FGunnerTargetData_SoundBase::StaticStruct(); }

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		Ar << SoundBase;
		Ar << ContextComponent;
		Ar << ContextBoneName;
		return bOutSuccess;
	}

public:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<USoundBase> SoundBase;
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<USceneComponent> ContextComponent;
	UPROPERTY(BlueprintReadOnly)
	FName ContextBoneName;
};


template <>
struct TStructOpsTypeTraits<FGunnerTargetData_SoundBase> : public TStructOpsTypeTraitsBase2<FGunnerTargetData_SoundBase>
{
	enum
	{
		WithNetSerializer = true
	};
};
