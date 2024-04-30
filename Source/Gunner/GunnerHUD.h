// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GunnerHUD.generated.h"

/**
 * 
 */
UCLASS()
class GUNNER_API AGunnerHUD : public AHUD
{
	GENERATED_BODY()
public:
	void SetupHUD();


public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> OverlayWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UUserWidget> OverlayWidget;
};
