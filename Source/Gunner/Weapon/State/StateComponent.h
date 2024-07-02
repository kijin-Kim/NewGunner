// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StateComponent.generated.h"


class AGunnerCharacter;
class AWeapon;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStateComponent();
	void EnterNewState(TSubclassOf<UStateComponent> NewState);
	virtual void Enter();
	virtual void Exit();

	AWeapon* GetWeapon() const;
	AGunnerCharacter* GetGunnerCharacter() const;
};
