// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gunner/Core/Event/EventCallbackBindInterface.h"
#include "Gunner/Core/Event/EventManagerComponent.h"
#include "CameraControlComponent.generated.h"

struct FInputMessage;


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UCameraControllerComponent : public UActorComponent, public IEventCallbackBindInterface
{
	GENERATED_BODY()

public:
	UCameraControllerComponent();
	virtual void InitializeComponent() override;

protected:
	virtual TArray<FEventCallbackHandle> SetupEvents() override;
	virtual UEventManagerComponent* GetEventManagerComponent() const override;

private:
	UFUNCTION()
	void OnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController);
	void Look(FGameplayTag GameplayTag, const FInputMessage& InputMessage);

private:
	UPROPERTY(EditDefaultsOnly)
	float BaseTurnRate = 45.0f;
	UPROPERTY(EditDefaultsOnly)
	float MouseSensitivity = 1.0f;

	TArray<FEventCallbackHandle> BoundedEventCallbackHandles;
};
