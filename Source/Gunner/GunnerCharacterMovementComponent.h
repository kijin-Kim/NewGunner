// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GunnerCharacterMovementComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UGunnerCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UGunnerCharacterMovementComponent();
	virtual bool CanAttemptJump() const override;
	virtual float GetMaxSpeed() const override;
};
