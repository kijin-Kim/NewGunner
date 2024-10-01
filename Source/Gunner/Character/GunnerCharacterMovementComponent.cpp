// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerCharacterMovementComponent.h"

#include "GunnerCharacter.h"
#include "Gunner/Gunner.h"
#include "Gunner/Core/Event/GunnerEventManagerComponent.h"
#include "Gunner/Core/Input/GunnerEventMessage.h"


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

	bWantsInitializeComponent = true;
}


void UGunnerCharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
	if (CharacterOwner)
	{
		CharacterOwner->ReceiveControllerChangedDelegate.AddDynamic(this, &UGunnerCharacterMovementComponent::OnControllerChanged);
	}
}

bool UGunnerCharacterMovementComponent::CanAttemptJump() const
{
	return IsJumpAllowed() && (IsMovingOnGround() || IsFalling());
}

float UGunnerCharacterMovementComponent::GetMaxSpeed() const
{
	float MaxSpeed = Super::GetMaxSpeed();

	const AGunnerCharacter* GunnerCharacterOwner = Cast<AGunnerCharacter>(PawnOwner);
	if (GunnerCharacterOwner)
	{
		if (!GunnerCharacterOwner->IsRunning())
		{
			MaxSpeed *= 0.6f;
		}
	}

	return MaxSpeed;
}

TArray<FGunnerEventCallbackHandle> UGunnerCharacterMovementComponent::SetupEvents()
{
	if (UGunnerEventManagerComponent* EventManagerComponent = GetEventManagerComponent())
	{
		return {
			EventManagerComponent->BindEventCallback<FGunnerEventMessage>(FGameplayTag::RequestGameplayTag(FName(TEXT("Input.Move"))), this, &ThisClass::Move),
			EventManagerComponent->BindEventCallback<FGunnerEventMessage>(FGameplayTag::RequestGameplayTag(FName(TEXT("Input.Jump"))), this, &ThisClass::Jump),
			EventManagerComponent->BindEventCallback<FGunnerEventMessage>(FGameplayTag::RequestGameplayTag(FName(TEXT("Input.Crouch"))), this, &ThisClass::CharacterCrouch),
			EventManagerComponent->BindEventCallback<FGunnerEventMessage>(FGameplayTag::RequestGameplayTag(FName(TEXT("Input.Uncrouch"))), this, &ThisClass::CharacterUncrouch)
		};
	}
	return {};
}

UGunnerEventManagerComponent* UGunnerCharacterMovementComponent::GetEventManagerComponent() const
{
	return CharacterOwner ? CharacterOwner->GetComponentByClass<UGunnerEventManagerComponent>() : nullptr;
}

void UGunnerCharacterMovementComponent::OnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController)
{
	if (NewController && OldController != NewController)
	{
		UnbindEvents();
		BindEvents();
	}
}

void UGunnerCharacterMovementComponent::Move(FGameplayTag GameplayTag, const FGunnerEventMessage& EventMessage)
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

void UGunnerCharacterMovementComponent::Jump(FGameplayTag GameplayTag, const FGunnerEventMessage& EventMessage)
{
	if (CharacterOwner)
	{
		CharacterOwner->Jump();
	}
}

void UGunnerCharacterMovementComponent::CharacterCrouch(FGameplayTag GameplayTag, const FGunnerEventMessage& EventMessage)
{
	if (CharacterOwner)
	{
		CharacterOwner->Crouch();
	}
}

void UGunnerCharacterMovementComponent::CharacterUncrouch(FGameplayTag GameplayTag, const FGunnerEventMessage& EventMessage)
{
	if (CharacterOwner)
	{
		CharacterOwner->UnCrouch();
	}
}
