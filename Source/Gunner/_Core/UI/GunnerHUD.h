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
	UFUNCTION(Exec)
	void LocalDebugTarget();
	void InitializeMainWidgets();

protected:
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<UUserWidget>> MainWidgetClasses;
};
