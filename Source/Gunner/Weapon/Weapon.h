// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gunner/Core/MultiPerspectiveMesh.h"
#include "Weapon.generated.h"


struct FWeaponData;
class UWeaponData;
class AGunnerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponEquipSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponUnequipSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPrimaryActionSignature, bool, bPressed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReloadActionSignature);


UCLASS()
class GUNNER_API AWeapon : public AActor, public IMultiPerspectiveMesh
{
	GENERATED_BODY()

	friend class UGunnerCheatManagerExtension;
public:
	AWeapon();
	virtual void BeginPlay() override;
	void OnPrimaryActionButtonPressed();
	void OnPrimaryActionButtonReleased();
	void OnReloadButtonPressed();
	void SetOwner(AActor* NewOwner) override;
	void OnRep_Owner() override;
	void Equip();
	void Unequip();

	virtual USkeletalMeshComponent* GetFirstPersonMeshComponent() const override { return FirstPersonMeshComponent; }
	virtual USkeletalMeshComponent* GetThirdPersonMeshComponent() const override { return ThirdPersonMeshComponent; }
	AGunnerCharacter* GetGunnerCharacterOwner() const;
	FName GetWeaponName() const { return WeaponName; }

private:
	void AttachMeshes();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName WeaponName = TEXT("NO_WEAPON");
	FName FPWeaponSocketName = TEXT("WeaponPoint");
	FName TPWeaponSocketName = TEXT("WeaponPoint");
	
	FWeaponData* WeaponData;
	
	UPROPERTY(BlueprintAssignable)
	FOnPrimaryActionSignature OnPrimaryActionDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnReloadActionSignature OnReloadActionDelegate;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> DefaultSceneRootComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<USkeletalMeshComponent> FirstPersonMeshComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<USkeletalMeshComponent> ThirdPersonMeshComponent;
	

private:
	UPROPERTY()
	mutable TObjectPtr<AGunnerCharacter> PrivateGunnerCharacterOwner;
};
