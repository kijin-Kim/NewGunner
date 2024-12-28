// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GunnerLobbyHUD.generated.h"

/**
 * 
 */
UCLASS()
class GUNNER_API AGunnerLobbyHUD : public AHUD
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
};
