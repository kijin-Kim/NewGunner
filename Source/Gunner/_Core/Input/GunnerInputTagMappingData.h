// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "GunnerInputTagMappingData.generated.h"

class UInputMappingContext;
class UInputAction;


USTRUCT()
struct FGunnerTriggerEventTagMapping
{
	FGunnerTriggerEventTagMapping()
		: TriggerEvent(ETriggerEvent::None)
		  , InputTag(FGameplayTag::EmptyTag)
	{
	}

	GENERATED_BODY()
	UPROPERTY(EditAnywhere)
	ETriggerEvent TriggerEvent;
	UPROPERTY(EditAnywhere)
	FGameplayTag InputTag;
};


USTRUCT()
struct GUNNER_API FGunnerInputTagMapping
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> InputAction;
	UPROPERTY(EditAnywhere)
	TArray<FGunnerTriggerEventTagMapping> TriggerEventTagMappings;
};

USTRUCT()
struct GUNNER_API FGunnerInputContextAndPriority
{
	GENERATED_BODY()

	FGunnerInputContextAndPriority()
		: InputMappingContext(nullptr)
		  , Priority(0)
	{
	}

	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputMappingContext> InputMappingContext;
	UPROPERTY(EditAnywhere)
	int32 Priority;
};


/**	
 * 
 */
UCLASS()
class GUNNER_API UGunnerInputTagMappingData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, meta = (TitleProperty = "InputAction"))
	TArray<FGunnerInputTagMapping> InputTagMappings;
	UPROPERTY(EditAnywhere, meta = (TitleProperty = "InputMappingContext"))
	TArray<FGunnerInputContextAndPriority> InputContextAndPriorities;
};
