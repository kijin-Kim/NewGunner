// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GunnerMinimapWidget.generated.h"

class UImage;
/**
 * 
 */
UCLASS(meta =(DisableNativeTick))
class GUNNER_API UGunnerMinimapWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> MinimapImage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInterface> MinimapMaterial;
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> MinimapDynamicMaterialInstance;
};
