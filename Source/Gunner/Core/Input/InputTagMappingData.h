// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "InputTagMappingData.generated.h"

class UInputMappingContext;
class UInputAction;


USTRUCT()
struct FTriggerEventTagMapping
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere)
	ETriggerEvent TriggerEvent;
	UPROPERTY(EditAnywhere)
	FGameplayTag InputTag;
};


USTRUCT()
struct GUNNER_API FInputTagMapping
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> InputAction;
	UPROPERTY(EditAnywhere)
	TArray<FTriggerEventTagMapping> TriggerEventTagMappings;
};

USTRUCT()
struct GUNNER_API FInputContextAndPriority
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputMappingContext> InputMappingContext;
	UPROPERTY(EditAnywhere)
	int32 Priority;
};


/**	
 * 
 */
UCLASS()
class GUNNER_API UInputTagMappingData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, meta = (TitleProperty = "InputAction"))
	TArray<FInputTagMapping> InputTagMappings;
	UPROPERTY(EditAnywhere, meta = (TitleProperty = "InputMappingContext"))
	TArray<FInputContextAndPriority> InputContextAndPriorities;
};
