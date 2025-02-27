// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Action/NexusAction.h"
#include "GunnerActionEquipmentBase.generated.h"

class UGunnerEquipmentDef;
class UGunnerEquipmentManagerComponent;
/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerActionEquipmentBase : public UNexusAction
{
	GENERATED_BODY()

public:
	virtual void OnActionAdded_Implementation() override;

private:
	UPROPERTY(BlueprintReadOnly, meta= (AllowPrivateAccess = "true"))
	TObjectPtr<UGunnerEquipmentManagerComponent> EquipmentManager;
	UPROPERTY(BlueprintReadOnly, meta= (AllowPrivateAccess = "true"))
	TObjectPtr<UGunnerEquipmentDef> EquipmentDef;
};
