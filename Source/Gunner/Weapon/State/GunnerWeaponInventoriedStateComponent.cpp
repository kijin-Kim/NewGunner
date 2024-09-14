// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerWeaponInventoriedStateComponent.h"

#include "Gunner/Gunner.h"

UGunnerWeaponInventoriedStateComponent::UGunnerWeaponInventoriedStateComponent()
{
	NetTransitionPolicy = EGunnerStateNetTransitionPolicy::ServerAuthoritative;
}

void UGunnerWeaponInventoriedStateComponent::OnEnter_Implementation()
{
	Super::OnEnter_Implementation();
	GR_LOG_SUB(LogGunner, Warning, TEXT("%s"), *GetOwner()->GetName());
}

void UGunnerWeaponInventoriedStateComponent::OnExit_Implementation()
{
	Super::OnExit_Implementation();
	GR_LOG_SUB(LogGunner, Warning, TEXT("%s"), *GetOwner()->GetName());
}
