// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Action/NexusAction.h"
#include "GunnerActionEquipmentBase.generated.h"

class AGunnerEquipment;
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
	UFUNCTION(BlueprintCallable)
	UGunnerEquipmentDef* GetEquipmentDef() const;
	UFUNCTION(BlueprintCallable)
	AGunnerEquipment* GetEquipment() const;

protected:
	virtual void InitializeAction(const FNexusActionDefHandle& InActionDefHandle, TWeakPtr<FNexusAgentInfo> InAgentInfo) override;

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UGunnerEquipmentManagerComponent> EquipmentManager;
};
