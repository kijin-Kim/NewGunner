// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVM/NexusActionViewModelContextResolver.h"
#include "GunnerViewModel_HealthIndicator.generated.h"

/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerViewModel_HealthIndicator : public UNexusActionViewModelBase
{
	GENERATED_BODY()

public:
	virtual void OnCreateViewModel(const UUserWidget* UserWidget) override;
	virtual void OnDestroyViewModel(const UObject* Object, const UMVVMView* View) const override;

	void SetHealth(int32 InHealth)
	{
		if (UE_MVVM_SET_PROPERTY_VALUE(Health, InHealth))
		{
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthIndicatorText);
		}
	}

	UFUNCTION(BlueprintPure, FieldNotify)
	FText GetHealthIndicatorText() const
	{
		return FText::AsNumber(Health);
	}

private:
	UFUNCTION()
	void OnHealthChanged(float OldValue, float NewValue);


protected:
	UPROPERTY(BlueprintReadOnly, FieldNotify)
	int32 Health;

};
