// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gunner/Core/StateMachine/GunnerStateComponent.h"
#include "GunnerWeaponIdleStateComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UGunnerWeaponIdleStateComponent : public UGunnerStateComponent
{
	GENERATED_BODY()
public:
	UGunnerWeaponIdleStateComponent();
	
	virtual void OnEnter_Implementation() override;
	virtual void OnExit_Implementation() override;
};
