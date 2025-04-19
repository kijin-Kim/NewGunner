// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "Gunner/Slot/GunnerSlotItem.h"
#include "MVVM/NexusActionViewModelContextResolver.h"
#include "GunnerSlotManagerViewModel.generated.h"

/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerSlotManagerViewModelContextResolver : public UNexusActionViewModelContextResolver
{
	GENERATED_BODY()

public:
	virtual UObject* CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const override;
};

UCLASS()
class GUNNER_API UGunnerSlotManagerViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	static UGunnerSlotManagerViewModel* CreateInstance(UNexusActionComponent* ActionComponent, const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View);


	
	void Init(UNexusActionComponent* InActionComponent);

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
	UPROPERTY()
	TObjectPtr<UNexusActionComponent> ActionComponent;

	UPROPERTY(BlueprintReadOnly, FieldNotify)
	int32 BulletCount;
	UPROPERTY(BlueprintReadOnly, FieldNotify)
	int32 MagazineBulletCount;
};
