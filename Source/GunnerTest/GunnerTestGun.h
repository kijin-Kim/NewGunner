// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gunner/Slot/GunnerActionSlotActivation.h"
#include "Gunner/Slot/GunnerGun.h"
#include "GunnerTestGun.generated.h"

UCLASS()
class GUNNERTESTS_API UGunnerTestActionActivatePrimary : public UGunnerActionSlotActivation
{
	GENERATED_BODY()

public:
	UGunnerTestActionActivatePrimary();
};

UCLASS()
class GUNNERTESTS_API UGunnerTestActionTransient : public UNexusAction
{
	GENERATED_BODY()
};



UCLASS()
class GUNNERTESTS_API AGunnerTestGun : public AGunnerGun
{
	GENERATED_BODY()

public:
	AGunnerTestGun();
	virtual void OnRemoved() override;

	const TArray<TSubclassOf<UNexusAction>>& GetPersistentActivationActions() const
	{
		return PersistentActivationActions;
	};

	const TArray<TSubclassOf<UNexusAction>>& GetTransientActivationActions() const
	{
		return TransientActivationActions;
	};

	const TArray<FNexusActionDefHandle>& GetPersistentActivationActionHandles() const
	{
		return PersistentActivationActionHandles;
	};

	const TArray<FNexusActionDefHandle>& GetTransientActivationActionHandles() const
	{
		return TransientActivationActionHandles;
	}

public:
	UPROPERTY()
	TObjectPtr<AActor> LastAgentActor;
};
