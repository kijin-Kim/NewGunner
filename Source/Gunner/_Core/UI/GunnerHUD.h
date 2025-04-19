// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Gunner/Slot/GunnerSlotItem.h"
#include "GunnerHUD.generated.h"

class UGunnerOverlayWidget;
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

private:
	void InitViewModel(UGunnerUserWidget* SlotWidget, AGunnerSlotItem* Item);
	UFUNCTION()
	void OnSlotItemAcquired(AGunnerSlotItem* Item);
	UFUNCTION()
	void OnSlotItemRemoved(AGunnerSlotItem* Item);
	UFUNCTION()
	void HandleSlotIndexDirty(float OldValue, float NewValue);

	void SetSlotWidgetVisibility(EGunnerSlotType SlotType, ESlateVisibility Visibility);

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGunnerOverlayWidget> OverlayWidgetClass;
	UPROPERTY()
	TObjectPtr<UGunnerOverlayWidget> OverlayWidget;


private:
	UPROPERTY()
	TMap<EGunnerSlotType, TObjectPtr<UUserWidget>> SlotWidgets;
};
