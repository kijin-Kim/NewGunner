// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputMessage.h"
#include "Components/ActorComponent.h"
#include "../Event/EventManagerComponent.h"
#include "InputEventDispatcherComponent.generated.h"


class UInputTagMappingData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UInputEventDispatcherComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInputEventDispatcherComponent();
	virtual void InitializeComponent() override;
	void OnInputEvent(UEnhancedInputComponent* InputComponent, FGameplayTag InputTag);
	UFUNCTION()
	void SetupInputEvent(APawn* OldPawn, APawn* NewPawn);

private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputTagMappingData> InputTagMappingData;
	FEventCallbackHandle Handle;
	UPROPERTY()
	APlayerController* PlayerController;
};
