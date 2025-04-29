// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Gunner/Item/GunnerSlotItem.h"
#include "GunnerView_EquipmentInventory.generated.h"

class UTextBlock;
class UImage;
class UVerticalBox;

UCLASS()
class GUNNER_API UGunnerEquipmentInventoryItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetImageAndText(const FSlateBrush& IconBrush, const FText& DisplayName);
	void ClearImageAndText();
	void PlayFadeOutAnimation();
	void PlayHighlightedFadeOutAnimation();
	

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemIconImage;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DisplayNameTextBlock;
	
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> FadeOutAnimation;
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> HighlightedFadeOutAnimation;
};


/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerView_EquipmentInventory : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void OnEquipmentItemChanged(const TMap<EGunnerSlotType, FGunnerEquipmentHudDataEntry>& EquipmentHudData);
	UFUNCTION(BlueprintCallable)
	void OnEquippedSlotTypeChanged(EGunnerSlotType SlotType);


protected:

	UPROPERTY(BlueprintReadWrite)
	TMap<EGunnerSlotType, TObjectPtr<UUserWidget>> EquipmentWidgets;
};
