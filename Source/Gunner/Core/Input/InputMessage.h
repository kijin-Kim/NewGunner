// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "InputMessage.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct GUNNER_API FInputMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FInputActionValue Value;
};
