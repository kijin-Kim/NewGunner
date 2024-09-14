// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerStateMachineTransitionCondition.h"


void UGunnerStateMachineTransitionCondition::OnEnter_Implementation()
{
}

void UGunnerStateMachineTransitionCondition::OnUpdate_Implementation(float DeltaTime)
{
}

void UGunnerStateMachineTransitionCondition::OnExit_Implementation()
{
	bShouldTransit = false;
}

bool UGunnerStateMachineTransitionCondition::ShouldTransit() const
{
	return bShouldTransit;
}

AActor* UGunnerStateMachineTransitionCondition::GetOwner() const
{
	return Cast<AActor>(GetOuter());
}
