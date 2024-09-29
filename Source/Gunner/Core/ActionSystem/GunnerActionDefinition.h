// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerActionDefinitionHandle.h"
#include "GunnerActionDefinition.generated.h"

/**
 * 
 */

class UGunnerAction;



USTRUCT()
struct FGunnerActionDefinition
{
	GENERATED_BODY()

	FGunnerActionDefinition();
	FGunnerActionDefinition(UObject* InSourceObject, TSubclassOf<UGunnerAction> InActionClass);
	bool operator==(const FGunnerActionDefinition& Other) const;
	bool operator!=(const FGunnerActionDefinition& Other) const;

	UPROPERTY()
	FGunnerActionDefinitionHandle Handle;
	UPROPERTY()
	TWeakObjectPtr<UObject> SourceObject;
	UPROPERTY()
	TSubclassOf<UGunnerAction> ActionClass;

	UPROPERTY()
	TObjectPtr<UGunnerAction> ActionCDO;

	UPROPERTY(NotReplicated)
	TObjectPtr<UGunnerAction> ActionInstance;
};

