// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Event/NexusEventBindHelperInterface.h"
#include "Action/SubComponent/NexusEventManagerComponent.h"
#include "CameraControlComponent.generated.h"

struct FNexusEventMessage;


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UCameraControllerComponent : public UActorComponent, public INexusEventBindHelperInterface
{
	GENERATED_BODY()

public:
	UCameraControllerComponent();
	void InitCameraController();

protected:
	virtual TArray<FNexusEventCallbackHandle> SetupEvents() override;

private:
	void Look(FGameplayTag GameplayTag, const FNexusEventMessage& EventMessage);


private:
	UPROPERTY(EditDefaultsOnly)
	float BaseTurnRate = 45.0f;
	UPROPERTY(EditDefaultsOnly)
	float MouseSensitivity = 1.0f;
};
