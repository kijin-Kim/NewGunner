// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"


class UWeaponData;
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

	USkeletalMeshComponent* GetFirstPersonMeshComponent() const { return FirstPersonMeshComponent; }

	

	

private:
	void AttachMeshes();

public:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UWeaponData> WeaponData;


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
