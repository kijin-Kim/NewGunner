// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerWeaponEquippingStateComponent.h"
#include "Gunner/Gunner.h"


UGunnerWeaponEquippingStateComponent::UGunnerWeaponEquippingStateComponent()
{
	NetTransitionPolicy = EGunnerStateNetTransitionPolicy::ServerAuthoritative;
}

void UGunnerWeaponEquippingStateComponent::OnEnter_Implementation()
{
	Super::OnEnter_Implementation();
	GR_LOG_SUB(LogGunner, Warning, TEXT("%s"), *GetOwner()->GetName());
}

void UGunnerWeaponEquippingStateComponent::OnExit_Implementation()
{
	Super::OnExit_Implementation();
	GR_LOG_SUB(LogGunner, Warning, TEXT("%s"), *GetOwner()->GetName());
}
