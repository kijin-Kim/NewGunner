// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Gunner/_Core/Event/GunnerEventCallbackBindInterface.h"
#include "Gunner/_Core/Event/GunnerEventManagerComponent.h"
#include "Gunner/_Core/Input/GunnerEventMessage.h"
#include "GunnerCharacterMovementComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UGunnerCharacterMovementComponent : public UCharacterMovementComponent, public IGunnerEventCallbackBindInterface
{
	GENERATED_BODY()

public:
	UGunnerCharacterMovementComponent();
	//~ Begin UCharacterMovementComponent Interface.
	virtual void InitializeComponent() override;
	virtual bool CanAttemptJump() const override;
	virtual float GetMaxSpeed() const override;
	//~ End UCharacterMovementComponent Interface.

protected:
	//~ Begin IGunnerEventCallbackBindInterface Interface.
	virtual TArray<FGunnerEventCallbackHandle> SetupEvents() override;
	virtual UGunnerEventManagerComponent* GetEventManagerComponent() const override;
	//~ End IGunnerEventCallbackBindInterface Interface.

	
private:
	UFUNCTION()
	void OnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController);
	
	void Move(FGameplayTag GameplayTag, const FGunnerEventMessage& EventMessage);
	void Jump(FGameplayTag GameplayTag, const FGunnerEventMessage& EventMessage);
	void CharacterCrouch(FGameplayTag GameplayTag, const FGunnerEventMessage& EventMessage);
	void CharacterUncrouch(FGameplayTag GameplayTag, const FGunnerEventMessage& EventMessage);

private:
	TArray<FGunnerEventCallbackHandle> BoundedEventCallbackHandles;
};
