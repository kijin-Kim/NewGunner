// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/GunnerUserWidget.h"
#include "GunnerOverlayWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerOverlayWidget : public UGunnerUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TimeText;
	
};
