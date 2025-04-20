// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Gunner/Slot/GunnerSlotItem.h"
#include "GunnerHUD.generated.h"

class UGunnerInventoryManagerComponent;
class UGunnerOverlayWidget;
class UGunnerUserWidget;

USTRUCT()
struct FGunnerSlotWidgetContainer
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<TObjectPtr<UUserWidget>> Widgets;
};


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
	void OnSlotItemActivated(EGunnerSlotType SlotType);
	void OnSlotItemDeactivated(EGunnerSlotType SlotType);
	UFUNCTION()
	void HandleSlotIndexDirty(float OldValue, float NewValue);

private:
	UGunnerInventoryManagerComponent* GetInventoryManagerComponentChecked() const;


public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGunnerOverlayWidget> OverlayWidgetClass;
	UPROPERTY()
	TObjectPtr<UGunnerOverlayWidget> OverlayWidget;

private:
	UPROPERTY()
	TMap<EGunnerSlotType, FGunnerSlotWidgetContainer> SlotTypeWidgetMap;
};
