// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVM/NexusActionViewModelContextResolver.h"
#include "GunnerInventoryViewModelBase.generated.h"

/**
 * 
 */

class UGunnerInventoryManagerComponent;

UCLASS()
class GUNNER_API UGunnerInventoryViewModelBase : public UNexusActionViewModelBase
{
	GENERATED_BODY()

public:
	virtual void OnCreateViewModel(const UUserWidget* UserWidget) override;

protected:
	UPROPERTY()
	TObjectPtr<UGunnerInventoryManagerComponent> InventoryManagerComponent;

};
