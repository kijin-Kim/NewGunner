// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gunner/Core/GunnerCharacterComponent.h"
#include "WeaponManagerComponent.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponChangedSignature, AWeapon*, LastWeapon, AWeapon*, NewWeapon);


class AGunnerCharacter;
class AWeapon;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UWeaponManagerComponent : public UGunnerCharacterComponent
{
	GENERATED_BODY()

	friend class UGunnerCheatManagerExtension;

public:
	UWeaponManagerComponent();
	virtual void BeginPlay() override;
	void OnReloadButtonPressed();
	void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void InitializeComponent() override;

	void SetupWeaponManager();
	void ChangeCurrentWeapon(uint32 WeaponIndex);

	AWeapon* GetCurrentWeapon() const { return CurrentWeapon; }

	void OnPrimaryButtonPressed();
	void OnPrimaryButtonReleased();

private:
	AWeapon* SpawnWeaponByClass(TSubclassOf<AWeapon> WeaponClass);
	UFUNCTION()
	void OnRep_Weapons();
	UFUNCTION()
	void OnRep_CurrentWeapon(AWeapon* LastWeapon);

	bool CanChangeCurrentWeapon(uint32 WeaponIndex) const;
	void LocalChangeCurrentWeapon(uint32 WeaponIndex);
	UFUNCTION(Server, Reliable)
	void ServerChangeCurrentWeapon(uint32 WeaponIndex);




public:
	UPROPERTY(BlueprintAssignable)
	FOnWeaponChangedSignature OnWeaponChangedDelegate;
	
	UPROPERTY(EditAnywhere, Category = "Weapon")
	TSubclassOf<AWeapon> DefaultPrimaryWeaponClass;
	UPROPERTY(EditAnywhere, Category = "Weapon")
	TSubclassOf<AWeapon> DefaultSecondaryWeaponClass;
	UPROPERTY(EditAnywhere, Category = "Weapon")
	TSubclassOf<AWeapon> DefaultMeleeWeaponClass;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> PrimaryAction;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> ReloadAction;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> PrimaryWeaponEquipAction;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> SecondaryWeaponEquipAction;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> MeleeWeaponEquipAction;
	


protected:
	UPROPERTY(ReplicatedUsing = OnRep_Weapons)
	TArray<TObjectPtr<AWeapon>> Weapons;
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentWeapon)
	TObjectPtr<AWeapon> CurrentWeapon;
	UPROPERTY()
	TObjectPtr<AGunnerCharacter> GunnerCharacterOwner;
	
};
