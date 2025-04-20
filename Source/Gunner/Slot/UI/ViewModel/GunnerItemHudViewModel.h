// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerInventoryManagerViewModel.h"
#include "GunnerItemHudViewModel.generated.h"

class AGunnerItem;
class UGunnerSlotItemHudComponent;

USTRUCT(BlueprintType)
struct FGunnerItemHudDataEntry
{
	GENERATED_BODY()

public:
	FGunnerItemHudDataEntry()
	{
		Reset();
	}

	explicit FGunnerItemHudDataEntry(const FText& InItemNames, UTexture2D* InStandardIconTextures, UTexture2D* InSimplifiedIconTextures)
		: ItemNames(InItemNames)
		  , StandardIconTextures(InStandardIconTextures)
		  , SimplifiedIconTextures(InSimplifiedIconTextures)
	{
	}

	void Reset()
	{
		ItemNames = FText::GetEmpty();
		StandardIconTextures = nullptr;
		SimplifiedIconTextures = nullptr;
	}

public:
	UPROPERTY(BlueprintReadOnly)
	FText ItemNames;
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UTexture2D> StandardIconTextures;
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UTexture2D> SimplifiedIconTextures;
};


/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerItemHudViewModel : public UGunnerInventoryManagerViewModel
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

protected:
	UPROPERTY(BlueprintReadOnly, FieldNotify)
	TArray<FGunnerItemHudDataEntry> ItemHudData;
};
