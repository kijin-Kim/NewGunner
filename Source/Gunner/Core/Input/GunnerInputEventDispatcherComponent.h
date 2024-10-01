// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "GunnerInputEventDispatcherComponent.generated.h"


struct FInputActionValue;
class UGunnerInputTagMappingData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UGunnerInputEventDispatcherComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGunnerInputEventDispatcherComponent();
	virtual void InitializeComponent() override;
	void OnInputEvent(const FInputActionValue& InputActionValue, UEnhancedInputComponent* InputComponent, FGameplayTag InputTag);
	UFUNCTION()
	void SetupInputEvent(APawn* OldPawn, APawn* NewPawn);

private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UGunnerInputTagMappingData> InputTagMappingData;
	UPROPERTY()
	APlayerController* PlayerController;
};
