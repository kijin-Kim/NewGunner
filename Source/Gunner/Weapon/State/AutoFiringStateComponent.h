// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FiringStateComponent.h"
#include "AutoFiringStateComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UAutoFiringStateComponent : public UFiringStateComponent
{
	GENERATED_BODY()

public:
	UAutoFiringStateComponent();
	UFUNCTION()
	void OnPrimaryAction(bool bPressed);
	
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void LocalFire() override;
	
	bool CanFire() const;

private:
	FTimerHandle FireFinishTimer;
};
