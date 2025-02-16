// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NexusEventMessage.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Event/NexusEventInterface.h"
#include "Event/NexusEventManagerComponent.h"
#include "GunnerCharacterMovementComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UGunnerCharacterMovementComponent : public UCharacterMovementComponent, public INexusEventInterface
{
	GENERATED_BODY()

public:
	UGunnerCharacterMovementComponent();
	//~ Begin UCharacterMovementComponent Interface.
	virtual bool CanAttemptJump() const override;
	virtual float GetMaxSpeed() const override;
	//~ End UCharacterMovementComponent Interface.
	
	void InitEvents();

protected:
	//~ Begin IGunnerEventCallbackBindInterface Interface.
	virtual TArray<FNexusEventCallbackHandle> SetupEvents() override;
	//~ End IGunnerEventCallbackBindInterface Interface.

	
private:

	
	void Move(FGameplayTag GameplayTag, const FNexusEventMessage& EventMessage);
	void Jump(FGameplayTag GameplayTag, const FNexusEventMessage& EventMessage);
	void CharacterCrouch(FGameplayTag GameplayTag, const FNexusEventMessage& EventMessage);
	void CharacterUncrouch(FGameplayTag GameplayTag, const FNexusEventMessage& EventMessage);

private:
	TArray<FNexusEventCallbackHandle> BoundedEventCallbackHandles;
};
