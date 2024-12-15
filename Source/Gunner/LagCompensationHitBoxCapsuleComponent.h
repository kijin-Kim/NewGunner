// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "LagCompensationHitBoxCapsuleComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API ULagCompensationHitBoxCapsuleComponent : public UCapsuleComponent
{
	GENERATED_BODY()

public:
	ULagCompensationHitBoxCapsuleComponent();
	void SetBoneName(FName InBoneName) { BoneName = InBoneName; }
	FName GetBoneName() const { return BoneName; }


private:
	FName BoneName;
};
