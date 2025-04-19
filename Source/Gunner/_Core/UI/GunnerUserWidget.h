// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GunnerUserWidget.generated.h"

class UGunnerUserWidgetController;
/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitUserWidget(UGunnerUserWidgetController* InController);
	UFUNCTION(BlueprintCallable)
	UGunnerUserWidgetController* GetUserWidgetController() const;

protected:
	UPROPERTY()
	TObjectPtr<UGunnerUserWidgetController> UserWidgetController;
};
