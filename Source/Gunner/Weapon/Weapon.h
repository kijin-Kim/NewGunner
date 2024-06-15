// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gunner/Core/AnimMontagePlayerInterface.h"
#include "Weapon.generated.h"


class UAnimMontagePlayerComponent;
struct FWeaponData;
class UWeaponData;
class AGunnerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponEquipSignature);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponUnequipSignature);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPrimaryActionSignature, bool, bPressed);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReloadActionSignature);


UCLASS()
class GUNNER_API AWeapon : public AActor, public IAnimMontagePlayerInterface
{
	GENERATED_BODY()

	friend class UGunnerCheatManagerExtension;

public:
	AWeapon();
	void OnPrimaryActionButtonPressed();
	void OnPrimaryActionButtonReleased();
	void OnReloadButtonPressed();
	void SetOwner(AActor* NewOwner) override;
	void OnRep_Owner() override;
	void Equip();
	void Unequip();

	virtual UAnimMontagePlayerComponent* GetAnimMontagePlayer_Implementation() override;
	virtual USkeletalMeshComponent* GetFirstPersonMeshComponent_Implementation() const override { return FirstPersonMeshComponent; }
	virtual USkeletalMeshComponent* GetThirdPersonMeshComponent_Implementation() const override { return ThirdPersonMeshComponent; }
	AGunnerCharacter* GetGunnerCharacterOwner() const;
	FName GetWeaponName() const { return WeaponName; }
	FWeaponData* GetWeaponData() const;

private:
	void AttachMeshes();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName WeaponName = TEXT("NO_WEAPON");
	FName FPWeaponSocketName = TEXT("WeaponPoint");
	FName TPWeaponSocketName = TEXT("WeaponPoint");

	mutable FWeaponData* WeaponDataCache;

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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontagePlayerComponent> AnimMontagePlayerComponent;

private:
	UPROPERTY()
	mutable TObjectPtr<AGunnerCharacter> PrivateGunnerCharacterOwner;
};
