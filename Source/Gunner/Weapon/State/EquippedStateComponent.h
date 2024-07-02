// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateComponent.h"
#include "EquippedStateComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UEquippedStateComponent : public UStateComponent
{
	GENERATED_BODY()

public:
	UEquippedStateComponent();
	UFUNCTION()
	void OnPrimaryAction(bool bPressed);
	
	UFUNCTION()
	void OnReloadAction();
	virtual void Enter() override;
	virtual void Exit() override;

private:
	bool CanFire() const;
	bool CanReload() const;
};
