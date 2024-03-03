// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"


class AGunnerCharacterBase;

UCLASS()
class GUNNER_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	AWeapon();
	void SetOwner(AActor* NewOwner) override;
	void OnRep_Owner() override;
	void Equip();
	void Unequip();
	AGunnerCharacterBase* GetGunnerCharacterOwner() const;

private:
	void AttachMeshes();

public:
	UPROPERTY(EditDefaultsOnly)
	FName FirstPersonWeaponSocketName = TEXT("R_WeaponPointSocket");
	UPROPERTY(EditDefaultsOnly)
	FName ThirdPersonWeaponSocketName = TEXT("WeaponPoint");

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> DefaultSceneRootComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<USkeletalMeshComponent> FirstPersonMeshComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<USkeletalMeshComponent> ThirdPersonMeshComponent;
	

	

private:
	UPROPERTY()
	mutable TObjectPtr<AGunnerCharacterBase> PrivateGunnerCharacterOwner;
};
