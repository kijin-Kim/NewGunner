// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Gunner/Core/Event/EventCallbackBindInterface.h"
#include "Gunner/Core/Event/EventManagerComponent.h"
#include "Gunner/Core/Input/GunnerEventMessage.h"
#include "GunnerCharacterMovementComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UGunnerCharacterMovementComponent : public UCharacterMovementComponent, public IEventCallbackBindInterface
{
	GENERATED_BODY()

public:
	UGunnerCharacterMovementComponent();
	virtual void InitializeComponent() override;
	virtual bool CanAttemptJump() const override;
	virtual float GetMaxSpeed() const override;

protected:
	virtual TArray<FEventCallbackHandle> SetupEvents() override;
	virtual UEventManagerComponent* GetEventManagerComponent() const override;

	
private:
	UFUNCTION()
	void OnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController);
	

	void Move(FGameplayTag GameplayTag, const FGunnerEventMessage& EventMessage);
	void Jump(FGameplayTag GameplayTag, const FGunnerEventMessage& EventMessage);
	void CharacterCrouch(FGameplayTag GameplayTag, const FGunnerEventMessage& EventMessage);
	void CharacterUncrouch(FGameplayTag GameplayTag, const FGunnerEventMessage& EventMessage);

private:
	TArray<FEventCallbackHandle> BoundedEventCallbackHandles;
};
