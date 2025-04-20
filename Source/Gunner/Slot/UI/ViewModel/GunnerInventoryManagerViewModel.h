// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVM/NexusActionViewModelContextResolver.h"
#include "GunnerInventoryManagerViewModel.generated.h"

/**
 * 
 */

class UGunnerInventoryManagerComponent;

UCLASS()
class GUNNER_API UGunnerInventoryManagerViewModel : public UNexusActionViewModel
{
	GENERATED_BODY()

public:
	virtual void OnCreateViewModel(const UUserWidget* UserWidget) override;

protected:
	UPROPERTY()
	TObjectPtr<UGunnerInventoryManagerComponent> InventoryManagerComponent;

};
