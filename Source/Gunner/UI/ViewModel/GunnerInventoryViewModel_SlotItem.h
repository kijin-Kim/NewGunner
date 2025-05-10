// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerInventoryViewModelBase.h"
#include "Blueprint/UserWidget.h"
#include "Gunner/Item/GunnerSlotItem.h"
#include "GunnerInventoryViewModel_SlotItem.generated.h"

class AGunnerItem;
class UGunnerSlotItemHudComponent;

USTRUCT(BlueprintType)
struct FGunnerSlotItemHudDataEntry
{
	GENERATED_BODY()

public:
	FGunnerSlotItemHudDataEntry()
	{
		Reset();
	}

	explicit FGunnerSlotItemHudDataEntry(const FText& InItemNames, const FSlateBrush& InStandardIconTextures, const FSlateBrush& InSimplifiedIconTextures)
		: ItemNames(InItemNames),
		  StandardIconBrush(InStandardIconTextures),
		  SimplifiedIconBrush(InSimplifiedIconTextures)
	{
	}

	void Reset()
	{
		ItemNames = FText::GetEmpty();
		StandardIconBrush = FSlateBrush();
		SimplifiedIconBrush = FSlateBrush();
	}

public:
	UPROPERTY(BlueprintReadOnly)
	FText ItemNames;
	UPROPERTY(BlueprintReadOnly)
	FSlateBrush StandardIconBrush;
	UPROPERTY(BlueprintReadOnly)
	FSlateBrush SimplifiedIconBrush;
};


/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerInventoryViewModel_SlotItem : public UGunnerInventoryViewModelBase
{
	GENERATED_BODY()

public:
	virtual void OnCreateViewModel(const UUserWidget* UserWidget) override;
	virtual void OnDestroyViewModel(const UObject* Object, const UMVVMView* View) const override;

private:
	UFUNCTION()
	void OnItemAcquired(AGunnerItem* Item);
	UFUNCTION()
	void OnItemRemoved(AGunnerItem* Item);
	UFUNCTION()
	void OnSlotIndexDirty(float OldValue, float NewValue);
	
	void SetCurrentAmountWidgetClass(const TSubclassOf<UUserWidget>& WidgetClass);

protected:
	UPROPERTY(BlueprintReadOnly, FieldNotify)
	TMap<EGunnerSlotType, FGunnerSlotItemHudDataEntry> SlotItemHudData;
	TMap<EGunnerSlotType, TSubclassOf<UUserWidget>> CachedSlotItemWidgetClasses;

	UPROPERTY(BlueprintReadOnly, FieldNotify)
	EGunnerSlotType ActiveSlotType = EGunnerSlotType::Num;
	UPROPERTY(BlueprintReadOnly, FieldNotify)
	TSubclassOf<UUserWidget> CurrentAmountWidgetClass;
};
