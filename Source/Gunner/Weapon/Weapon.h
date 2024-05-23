// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"


class UWeaponData;
class AGunnerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponEquipSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponUnequipSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPrimaryActionSignature, bool, bPressed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReloadActionSignature);


UCLASS()
class GUNNER_API AWeapon : public AActor
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
	AGunnerCharacter* GetGunnerCharacterOwner() const;
	USkeletalMeshComponent* GetFirstPersonMeshComponent() const { return FirstPersonMeshComponent; }
	FName GetWeaponName() const { return WeaponName; }

private:
	void AttachMeshes();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName WeaponName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	FName FPWeaponSocketName = TEXT("WeaponPoint");
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	FName TPWeaponSocketName = TEXT("WeaponPoint");
	

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson|Weapon")
	TObjectPtr<UAnimMontage> TPWeaponEquipMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson|Character")
	TSubclassOf<UAnimInstance> TPCharacterAnimInstance;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|ThirdPerson|Character")
	TObjectPtr<UAnimMontage> TPCharacterEquipMontage;
	
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
