// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerAnimInstance.h"

void UGunnerAnimInstance::SetLocomotionAnimSet(UGunnerLocomotionAnimSet* InLocomotionAnimSet)
{
	GameThreadLocomotionAnimSet = InLocomotionAnimSet;
}

void UGunnerAnimInstance::ClearLocomotionAnimSet()
{
	GameThreadLocomotionAnimSet = DefaultLocomotionAnimSet;
}

UGunnerLocomotionAnimSet* UGunnerAnimInstance::GetLocomotionAnimSet() const
{
	return GameThreadLocomotionAnimSet;
}

void UGunnerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	LocomotionAnimSet = GameThreadLocomotionAnimSet;
	LocomotionAnimSet = LocomotionAnimSet ? LocomotionAnimSet : DefaultLocomotionAnimSet;
}