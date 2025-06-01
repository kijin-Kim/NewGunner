// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Action/SubComponent/NexusEventManagerComponent.h"
#include "Blueprint/UserWidget.h"
#include "Event/NexusEventMessage.h"
#include "GunnerView_HitIndicatorOverlay.generated.h"

class UGunnerView_HitIndicator;
/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerView_HitIndicatorOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	void OnDamagedEvent(FGameplayTag GameplayTag, const FNexusEventMessage& EventMessage);

protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGunnerView_HitIndicator> HitIndicatorWidgetClass;

private:
	FNexusEventCallbackHandle EventCallbackHandle;
};
