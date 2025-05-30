// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerViewModelContextResolver.h"
#include "GunnerViewModel_HitIndicator.generated.h"

class AGunnerPlayerController;
/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerViewModel_HitIndicator : public UGunnerViewModelBase
{
	GENERATED_BODY()

public:
	virtual void OnCreateViewModel(const UUserWidget* UserWidget) override;
	virtual void OnDestroyViewModel(const UObject* Object, const UMVVMView* View) const override;

	void SetCauserLocation(const FVector& Location)
	{
		CauserLocation = Location;
	}

protected:
	void OnPlayerControlRotationChanged(const FRotator& Rotator);


protected:
	FVector CauserLocation;
	UPROPERTY(BlueprintReadOnly, FieldNotify)
	float RenderTransformAngle = 0.0f;


private:
	UPROPERTY()
	TObjectPtr<AGunnerPlayerController> PlayerController;
	UPROPERTY()
	TObjectPtr<AActor> VictimActor;
};
