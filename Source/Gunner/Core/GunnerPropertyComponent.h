// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GunnerPropertyComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UGunnerPropertyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y);
};
