// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerCheatManagerExtension.h"

#include "GameFramework/SpringArmComponent.h"
#include "Gunner/GunnerCharacterBase.h"
#include "Gunner/Weapon/Weapon.h"
#include "Gunner/Weapon/WeaponManagerComponent.h"

UGunnerCheatManagerExtension::UGunnerCheatManagerExtension()
{
#if UE_WITH_CHEAT_MANAGER
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		UCheatManager::RegisterForOnCheatManagerCreated(FOnCheatManagerCreated::FDelegate::CreateLambda([](UCheatManager* CheatManager)
		{
			CheatManager->AddCheatManagerExtension(NewObject<ThisClass>(CheatManager));
		}));
	}
#endif
}

void UGunnerCheatManagerExtension::ToggleThirdPersonMode()
{
	APlayerController* PC = GetPlayerController();
	if (!PC)
	{
		return;
	}

	AGunnerCharacterBase* GunnerCharacter = Cast<AGunnerCharacterBase>(PC->GetPawn());
	if (!GunnerCharacter)
	{
		return;
	}

	bIsTPMode = !bIsTPMode;
	
	GunnerCharacter->FirstPersonMeshComponent->SetHiddenInGame(bIsTPMode);
	GunnerCharacter->GetMesh()->SetOwnerNoSee(!bIsTPMode);
	GunnerCharacter->FirstPersonSpringArmComponent->TargetArmLength = bIsTPMode ? 500.0f : 0.0f;
	GunnerCharacter->FirstPersonSpringArmComponent->SocketOffset = bIsTPMode ? FVector{0.0f, 50.0f, 0.0f} : FVector::ZeroVector;
	
	if (GunnerCharacter->WeaponManagerComponent->CurrentWeapon) // TODO: Signaled By Weapon Change
	{
		GunnerCharacter->WeaponManagerComponent->CurrentWeapon->FirstPersonMeshComponent->SetHiddenInGame(bIsTPMode);
		GunnerCharacter->WeaponManagerComponent->CurrentWeapon->ThirdPersonMeshComponent->SetOwnerNoSee(!bIsTPMode);
		
	}
}
