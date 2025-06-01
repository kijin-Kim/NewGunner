// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Gunner/UI/ViewModel/GunnerViewModel_HitIndicator.h"
#include "GunnerView_HitIndicator.generated.h"

/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerView_HitIndicator : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<UGunnerViewModel_HitIndicator> HitIndicatorViewModel;
};
