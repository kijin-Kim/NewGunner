// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerInventoryViewModelBase.h"
#include "Gunner/Item/GunnerSlotItem.h"
#include "GunnerInventoryViewModel_Equipment.generated.h"

class AGunnerItem;
class UGunnerSlotItemHudComponent;

USTRUCT(BlueprintType)
struct FGunnerEquipmentHudDataEntry
{
	GENERATED_BODY()

public:
	FGunnerEquipmentHudDataEntry()
	{
		Reset();
	}

	explicit FGunnerEquipmentHudDataEntry(const FText& InItemNames, const FSlateBrush& InStandardIconTextures, const FSlateBrush& InSimplifiedIconTextures)
		: ItemNames(InItemNames)
		, StandardIconBrush(InStandardIconTextures)
	 		, SimplifiedIconBrush(InSimplifiedIconTextures)
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
class GUNNER_API UGunnerInventoryViewModel_Equipment : public UGunnerInventoryViewModelBase
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

protected:
	UPROPERTY(BlueprintReadOnly, FieldNotify)
	TMap<EGunnerSlotType, FGunnerEquipmentHudDataEntry> EquipmentHudData;
	UPROPERTY(BlueprintReadOnly, FieldNotify)
	EGunnerSlotType EquippedSlotType = EGunnerSlotType::Num;
};
