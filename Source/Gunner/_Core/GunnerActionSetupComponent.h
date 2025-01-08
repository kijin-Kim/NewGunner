// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GunnerActionSetupComponent.generated.h"


class UGunnerActionSet;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UGunnerActionSetupComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGunnerActionSetupComponent();
	void AuthSetupActionSets();

private:
	UPROPERTY(EditAnywhere)
	TArray<TObjectPtr<UGunnerActionSet>> ActionSets;
};
