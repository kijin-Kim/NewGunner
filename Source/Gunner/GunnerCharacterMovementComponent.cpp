// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerCharacterMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"


UGunnerCharacterMovementComponent::UGunnerCharacterMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

bool UGunnerCharacterMovementComponent::CanAttemptJump() const
{
	return IsJumpAllowed() && (IsMovingOnGround() || IsFalling());
}