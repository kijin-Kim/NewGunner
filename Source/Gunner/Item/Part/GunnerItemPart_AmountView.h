// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerItemPartBase.h"
#include "GunnerItemPart_AmountView.generated.h"

/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerItemPart_AmountView : public UGunnerItemPartBase
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> ViewClass;
};
