// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerAnimInstance.h"

#include "GunnerLocomotionAnimSet.h"

void UGunnerAnimInstance::SetLocomotionAnimSet(UGunnerLocomotionAnimSet* InLocomotionAnimSet)
{
	LocomotionAnimSet = InLocomotionAnimSet;
}

void UGunnerAnimInstance::ClearLocomotionAnimSet()
{
	LocomotionAnimSet = DefaultLocomotionAnimSet;
}

