// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "InputTagMappingData.generated.h"

class UInputMappingContext;
class UInputAction;

USTRUCT()
struct GUNNER_API FInputTagMapping
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FGameplayTag InputTag;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> InputAction;
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
	UPROPERTY(EditAnywhere)
	TArray<FInputTagMapping> InputTagMappings;
	UPROPERTY(EditAnywhere)
	TArray<FInputContextAndPriority> InputContextAndPriorities;
};
