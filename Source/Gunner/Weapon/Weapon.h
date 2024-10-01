// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gunner/Core/GunnerAnimMontagePlayerInterface.h"
#include "Weapon.generated.h"


class UDroppedStateComponent;
class UReloadingStateComponent;
class UFiringStateComponent;
class UEquippedStateComponent;
class UDrawingStateComponent;
class UInventoriedStateComponent;
class UGunnerAnimMontagePlayerComponent;
struct FWeaponData;
class AGunnerCharacter;
class UStateComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponEquipSignature);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponUnequipSignature);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPrimaryActionSignature, bool, bPressed);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReloadActionSignature);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponBulletCountChangedSignature, int32, BulletCount, int32, MagazineBulletCount);


UCLASS()
class GUNNER_API AWeapon : public AActor, public IGunnerAnimMontagePlayerInterface
{
	GENERATED_BODY()

	friend class UGunnerCheatManagerExtension;

public:
	AWeapon();
	virtual void PostInitializeComponents() override;
	void OnPrimaryActionButtonPressed();
	void OnPrimaryActionButtonReleased();
	void OnReloadButtonPressed();
	void Equip();
	void Unequip();

	virtual UGunnerAnimMontagePlayerComponent* GetAnimMontagePlayer_Implementation() override;
	virtual USkeletalMeshComponent* GetFirstPersonMeshComponent_Implementation() const override { return FirstPersonMeshComponent; }
	virtual USkeletalMeshComponent* GetThirdPersonMeshComponent_Implementation() const override { return ThirdPersonMeshComponent; }
	AGunnerCharacter* GetGunnerCharacterOwner() const;
	FName GetWeaponName() const { return WeaponName; }
	FWeaponData* GetWeaponData() const;

	UStaticMeshComponent* GetFirstPersonMagainzeMeshComponent() const { return FirstPersonMagazineMeshComponent; };
	UStaticMeshComponent* GetThirdPersonMagainzeMeshComponent() const { return ThirdPersonMagazineMeshComponent; };

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName WeaponName = TEXT("NO_WEAPON");

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> FirstPersonMagazineMeshComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> ThirdPersonMagazineMeshComponent;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UGunnerAnimMontagePlayerComponent> AnimMontagePlayerComponent;

private:
	UPROPERTY()
	mutable TObjectPtr<AGunnerCharacter> PrivateGunnerCharacterOwner;

public:
	UPROPERTY(BlueprintAssignable)
	FOnWeaponBulletCountChangedSignature OnWeaponBulletCountChangedDelegate;

	void SetBulletCount(int32 InBulletCount);
	int32 GetBulletCount() const;
	int32 GetMaxMagazineBulletCount() const;
	int32 MaxMagazineBulletCount = 30;
	int32 GetMaxBulletCount() const;
	int32 MaxBulletCount = 30;
	
	UPROPERTY(BlueprintReadOnly)
	int32 BulletCount = 30;
	void SetMagazineBulletCount(int32 InMagazineBulletCount);
	int32 GetMagazineBulletCount() const;
	UPROPERTY(BlueprintReadOnly)
	int32 MagazineBulletCount = 30;
	float GetFiringDelay() const;
	float FiringDelay =  1.0f / 9.75f;

	UPROPERTY()
	TObjectPtr<UStateComponent> CurrentState;


	void EnterNewState(TSubclassOf<UStateComponent> NewState);


	UPROPERTY(EditAnywhere)
	TSubclassOf<UStateComponent> InventoriedStateComponentClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<UStateComponent> DrawingStateComponentClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<UStateComponent> EquippedStateComponentClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<UStateComponent> FiringStateComponentClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<UStateComponent> ReloadingStateComponentClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<UStateComponent> DroppedStateComponentClass;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UInventoriedStateComponent> InventoriedStateComponent;
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UDrawingStateComponent> DrawingStateComponent;
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UEquippedStateComponent> EquippedStateComponent;
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UFiringStateComponent> FiringStateComponent;
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UReloadingStateComponent> ReloadingStateComponent;
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UDroppedStateComponent> DroppedStateComponent;
	
};
