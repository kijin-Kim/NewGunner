// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gunner/Core/GunnerCharacterComponent.h"
#include "WeaponManagerComponent.generated.h"


class AGunnerCharacter;
class AWeapon;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UWeaponManagerComponent : public UGunnerCharacterComponent
{
	GENERATED_BODY()

	friend class UGunnerCheatManagerExtension;

public:
	UWeaponManagerComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;

	void SetupWeaponManager();
	void ChangeCurrentWeapon(uint32 WeaponIndex);

	AWeapon* GetCurrentWeapon() const { return CurrentWeapon; }

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
	UPROPERTY(EditAnywhere, Category = "Weapon")
	TSubclassOf<AWeapon> DefaultPrimaryWeaponClass;
	UPROPERTY(EditAnywhere, Category = "Weapon")
	TSubclassOf<AWeapon> DefaultSecondaryWeaponClass;
	UPROPERTY(EditAnywhere, Category = "Weapon")
	TSubclassOf<AWeapon> DefaultMeleeWeaponClass;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_Weapons)
	TArray<TObjectPtr<AWeapon>> Weapons;
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentWeapon)
	TObjectPtr<AWeapon> CurrentWeapon;
	UPROPERTY()
	TObjectPtr<AGunnerCharacter> GunnerCharacterOwner;
};
