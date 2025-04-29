// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerItemPartBase.h"
#include "GunnerItemPart_Icon.generated.h"

/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerItemPart_Icon : public UGunnerItemPartBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FText DisplayNameText;
	UPROPERTY(EditAnywhere)
	FSlateBrush StandardIconBrush;
	UPROPERTY(EditAnywhere)
	FSlateBrush SimplifiedIconBrush;
};
