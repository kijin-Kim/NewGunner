// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GunnerHUD.generated.h"

class UGunnerUserWidget;
/**
 * 
 */
UCLASS()
class GUNNER_API AGunnerHUD : public AHUD
{
	GENERATED_BODY()
public:
	void SetupHUD(APlayerState* PlayerState);
	virtual void GetDebugActorList(TArray<AActor*>& InOutList) override;


public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGunnerUserWidget> OverlayWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UGunnerUserWidget> OverlayWidget;
};
