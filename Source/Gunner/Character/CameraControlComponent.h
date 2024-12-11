// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gunner/_Core/Event/GunnerEventCallbackBindInterface.h"
#include "Gunner/_Core/Event/GunnerEventManagerComponent.h"
#include "CameraControlComponent.generated.h"

struct FGunnerEventMessage;


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UCameraControllerComponent : public UActorComponent, public IGunnerEventCallbackBindInterface
{
	GENERATED_BODY()

public:
	UCameraControllerComponent();
	virtual void InitializeComponent() override;

protected:
	virtual TArray<FGunnerEventCallbackHandle> SetupEvents() override;

private:
	UFUNCTION()
	void OnPlayerStateChanged(APlayerState* OldPlayerState, APlayerState* NewPlayerState);
	void Look(FGameplayTag GameplayTag, const FGunnerEventMessage& EventMessage);

private:
	UPROPERTY(EditDefaultsOnly)
	float BaseTurnRate = 45.0f;
	UPROPERTY(EditDefaultsOnly)
	float MouseSensitivity = 1.0f;
};
