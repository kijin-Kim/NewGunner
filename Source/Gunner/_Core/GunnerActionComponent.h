// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NexusActionComponent.h"
#include "GunnerActionComponent.generated.h"


class UGunnerActionSet;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UGunnerActionComponent : public UNexusActionComponent
{
	GENERATED_BODY()

public:
	virtual void OnSetupActionComponent() override;

private:
	UPROPERTY(EditAnywhere)
	TArray<TObjectPtr<UGunnerActionSet>> ActionSets;
};
