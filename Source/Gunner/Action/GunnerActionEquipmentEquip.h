// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerActionEquipmentBase.h"
#include "GunnerActionEquipmentEquip.generated.h"

/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerActionEquipmentEquip : public UGunnerActionEquipmentBase
{
	GENERATED_BODY()

protected:
	virtual void InitializeAction(const FNexusActionDefHandle& InActionDefHandle, TWeakPtr<FNexusAgentInfo> InAgentInfo) override;
};
