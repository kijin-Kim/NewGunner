// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerCharacterMovementComponent.h"

#include "GunnerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"


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
