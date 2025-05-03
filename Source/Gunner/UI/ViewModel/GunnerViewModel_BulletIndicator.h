// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVM/NexusActionViewModelContextResolver.h"
#include "GunnerViewModel_BulletIndicator.generated.h"

/**
 * 
 */

UCLASS()
class GUNNER_API UGunnerViewModel_BulletIndicator : public UNexusActionViewModelBase
{
	GENERATED_BODY()

public:
	virtual void OnCreateViewModel(const UUserWidget* UserWidget) override;
	virtual void OnDestroyViewModel(const UObject* Object, const UMVVMView* View) const override;

	void SetBulletCount(int32 InBulletCount)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(BulletCount, InBulletCount))
		{
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetBulletCountText);
		}
	}

	void SetMagazineBulletCount(int32 InMagazineBulletCount)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(MagazineBulletCount, InMagazineBulletCount))
		{
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetMagazineBulletCountText);
		}
	}

	UFUNCTION(BlueprintPure, FieldNotify)
	FText GetBulletCountText() const
	{
		return FText::AsNumber(BulletCount);
	}

	UFUNCTION(BlueprintPure, FieldNotify)
	FText GetMagazineBulletCountText() const
	{
		return FText::AsNumber(MagazineBulletCount);
	}

private:
	UFUNCTION()
	void OnBulletCountChanged(float OldValue, float NewValue);
	UFUNCTION()
	void OnMagazineBulletCountChanged(float OldValue, float NewValue);

protected:
	UPROPERTY(BlueprintReadOnly, FieldNotify)
	int32 BulletCount;
	UPROPERTY(BlueprintReadOnly, FieldNotify)
	int32 MagazineBulletCount;
};
