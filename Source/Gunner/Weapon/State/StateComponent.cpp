// Fill out your copyright notice in the Description page of Project Settings.


#include "StateComponent.h"

#include "Gunner/Gunner.h"
#include "Gunner/Weapon/Weapon.h"


UStateComponent::UStateComponent()
{
}



void UStateComponent::EnterNewState(TSubclassOf<UStateComponent> NewState)
{
	AWeapon* WeaponOwner = Cast<AWeapon>(GetOwner());
	WeaponOwner->EnterNewState(NewState);
}

void UStateComponent::Enter()
{
	UE_LOG(LogGunner, Warning, TEXT(R"(/------------------------\)"));
	UE_LOG(LogGunner, Warning, TEXT("Enter : %s"), *GetClass()->GetName());

}

void UStateComponent::Exit()
{
	UE_LOG(LogGunner, Warning, TEXT("Exit : %s"), *GetClass()->GetName());
}

AWeapon* UStateComponent::GetWeapon() const
{
	return GetOwner<AWeapon>();
}

AGunnerCharacter* UStateComponent::GetGunnerCharacter() const
{
	return GetWeapon()->GetGunnerCharacterOwner();
}
