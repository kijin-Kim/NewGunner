// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Action/NexusActionComponent.h"
#include "GunnerTestActionComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNERTESTS_API UGunnerTestActionComponent : public UNexusActionComponent
{
	GENERATED_BODY()

public:
	virtual void OnSetupActionComponent() override;
};
