// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerWeaponIdleStateComponent.h"
#include "Gunner/Gunner.h"

UGunnerWeaponIdleStateComponent::UGunnerWeaponIdleStateComponent()
{
	NetTransitionPolicy = EGunnerStateNetTransitionPolicy::ServerAuthoritative;
}

void UGunnerWeaponIdleStateComponent::OnEnter_Implementation()
{
	Super::OnEnter_Implementation();
	GR_LOG_SUB(LogGunner, Warning, TEXT("%s"), *GetOwner()->GetName());
}

void UGunnerWeaponIdleStateComponent::OnExit_Implementation()
{
	Super::OnExit_Implementation();
	GR_LOG_SUB(LogGunner, Warning, TEXT("%s"), *GetOwner()->GetName());
}
