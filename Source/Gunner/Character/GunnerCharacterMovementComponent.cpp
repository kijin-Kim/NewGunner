// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerCharacterMovementComponent.h"

#include "GunnerCharacter.h"
#include "Event/NexusEventMessage.h"
#include "Action/NexusActionComponent.h"
#include "Event/NexusEventManagerComponent.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"


UGunnerCharacterMovementComponent::UGunnerCharacterMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	GravityScale = 2.0f;
	JumpZVelocity = 700.0f;
	MaxWalkSpeedCrouched = 250.0f;
	MaxWalkSpeed = 675.0f;
	NavAgentProps.bCanCrouch = true;

	BrakingFriction = 3000.0f;
	MaxAcceleration = 3675.0f;
	GroundFriction = 4.75f;
}

bool UGunnerCharacterMovementComponent::CanAttemptJump() const
{
	return IsJumpAllowed() && (IsMovingOnGround() || IsFalling());
}

float UGunnerCharacterMovementComponent::GetMaxSpeed() const
{
	return UNexusActionComponent::GetPropertyValueFromActor(CharacterOwner, FGameplayTag::RequestGameplayTag(FName(TEXT("Property.MaxSpeedMultiplier"))))
		* Super::GetMaxSpeed();
}

TArray<FNexusEventCallbackHandle> UGunnerCharacterMovementComponent::SetupEvents()
{
	if (UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(CharacterOwner))
	{
		return {
			ActionComponent->BindEventCallback<FNexusEventMessage>(TAG_Input_Move, this, &ThisClass::Move),
			ActionComponent->BindEventCallback<FNexusEventMessage>(TAG_Input_Jump, this, &ThisClass::Jump),
			ActionComponent->BindEventCallback<FNexusEventMessage>(TAG_Input_Crouch, this, &ThisClass::CharacterCrouch),
			ActionComponent->BindEventCallback<FNexusEventMessage>(TAG_Input_Uncrouch, this, &ThisClass::CharacterUncrouch)
		};
	}
	return {};
}

void UGunnerCharacterMovementComponent::InitEvents()
{
	UnbindEvents(UNexusActionComponent::GetActionComponentFromActor(CharacterOwner));
	BindEvents();
}

void UGunnerCharacterMovementComponent::Move(FGameplayTag GameplayTag, const FNexusEventMessage& EventMessage)
{
	AController* Controller = GetController();
	if (!Controller)
	{
		return;
	}

	if (CharacterOwner)
	{
		const FVector2D MovementVector = EventMessage.InputActionValue.Get<FVector2D>();
		AddInputVector(CharacterOwner->GetActorForwardVector() * MovementVector.Y, false);
		AddInputVector(CharacterOwner->GetActorRightVector() * MovementVector.X, false);
	}
}

void UGunnerCharacterMovementComponent::Jump(FGameplayTag GameplayTag, const FNexusEventMessage& EventMessage)
{
	if (CharacterOwner)
	{
		CharacterOwner->Jump();
	}
}

void UGunnerCharacterMovementComponent::CharacterCrouch(FGameplayTag GameplayTag, const FNexusEventMessage& EventMessage)
{
	if (CharacterOwner)
	{
		CharacterOwner->Crouch();
	}
}

void UGunnerCharacterMovementComponent::CharacterUncrouch(FGameplayTag GameplayTag, const FNexusEventMessage& EventMessage)
{
	if (CharacterOwner)
	{
		CharacterOwner->UnCrouch();
	}
}
