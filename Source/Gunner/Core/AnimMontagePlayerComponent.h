// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AnimMontagePlayerComponent.generated.h"



UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UAnimMontagePlayerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAnimMontagePlayerComponent();
	virtual void BeginPlay() override;
	float PlayMontage(UAnimMontage* AnimMontage, bool bIsThirdPerson, float InPlayRate = 1.f, FName StartSectionName = NAME_None);

private:
	float LocalPlayMontage(UAnimMontage* AnimMontage, bool bIsThirdPerson, float InPlayRate = 1.f, FName StartSectionName = NAME_None);
};
