// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVM/NexusActionViewModelContextResolver.h"
#include "GunnerBulletIndicatorViewModel.generated.h"

/**
 * 
 */

UCLASS(DisplayName = "Bullet Indicator ViewModel")
class GUNNER_API UGunnerBulletIndicatorViewModel : public UNexusActionViewModel
{
	GENERATED_BODY()

public:
	virtual void OnCreateViewModel(const UUserWidget* UserWidget) override;
	virtual void OnDestroyViewModel(const UObject* Object, const UMVVMView* View) const override;

	void SetBulletCount(int32 InBulletCount)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(BulletCount, InBulletCount))
		{
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetBulletIndicatorText);
		}
	}

	void SetMagazineBulletCount(int32 InMagazineBulletCount)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(MagazineBulletCount, InMagazineBulletCount))
		{
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetBulletIndicatorText);
		}
	}


	UFUNCTION(BlueprintPure, FieldNotify)
	FText GetBulletIndicatorText() const
	{
		return FText::Format(FText::FromString("{0}/{1}"), FText::AsNumber(BulletCount), FText::AsNumber(MagazineBulletCount));
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
