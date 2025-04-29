// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gunner/Item/GunnerGun.h"
#include "GunnerItem_Test.generated.h"



UCLASS()
class GUNNERTESTS_API AGunnerItem_Test : public AGunnerItem
{
	GENERATED_BODY()

public:
	AGunnerItem_Test();
	
	virtual void OnRemoved() override;

public:
	UPROPERTY()
	TObjectPtr<AActor> LastAgentActor;
};
