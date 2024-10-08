// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gunner/Core/GunnerAnimMontagePlayerInterface.h"
#include "GameFramework/Character.h"
#include "Gunner/Core/ActionSystem/GunnerActionComponentInterface.h"
#include "Gunner/Core/Event/GunnerEventManagerInterface.h"
#include "GunnerCharacter.generated.h"

class UGunnerAction;
class AGunnerEquipment;
class UGunnerEquipmentManagerComponent;
class UGunnerActionComponent;
class AWeapon;
class UCameraControllerComponent;
class UGunnerEventManagerComponent;
class UGunnerAnimMontagePlayerComponent;
class UHealthComponent;
class USpringArmComponent;
class UWeaponManagerComponent;
class UInputAction;
class UInputMappingContext;
class UCameraComponent;


UCLASS()
class GUNNER_API AGunnerCharacter : public ACharacter, public IGunnerAnimMontagePlayerInterface, public IGunnerActionComponentInterface, public IGunnerEventManagerInterface
{
	GENERATED_BODY()

	friend class UGunnerCheatManagerExtension;

public:
	AGunnerCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual bool CanJumpInternal_Implementation() const override;

	virtual UGunnerAnimMontagePlayerComponent* GetAnimMontagePlayer_Implementation() override;
	virtual USkeletalMeshComponent* GetFirstPersonMeshComponent_Implementation() const override { return FirstPersonMeshComponent; }
	virtual USkeletalMeshComponent* GetThirdPersonMeshComponent_Implementation() const override { return GetMesh(); }
	bool IsRunning() const;

	virtual UGunnerActionComponent* GetActionComponent() const override;
	virtual UGunnerEventManagerComponent* GetEventManagerComponent() const override;


	virtual void OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState) override;

private:
	void SetRunning(bool bNewRunning);
	UFUNCTION(Server, Reliable)
	void ServerRun(bool bNewRunning);

public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> LookAction;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> WalkAction;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<USkeletalMeshComponent> FirstPersonMeshComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> FirstPersonSpringArmComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UWeaponManagerComponent> WeaponManagerComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UGunnerAnimMontagePlayerComponent> AnimMontagePlayerComponent;
	UPROPERTY(BlueprintReadOnly)
	bool bIsRunning = true;

private:
	UPROPERTY()
	TObjectPtr<UCameraControllerComponent> CameraControllerComponent;

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> TestWeaponClass;
	UPROPERTY()
	TObjectPtr<AWeapon> TestWeapon;

public:
	virtual void PossessedBy(AController* NewController) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<AGunnerEquipment>> InitialEquipmentClasses;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<UGunnerAction>> InitialActions;


	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGunnerEquipmentManagerComponent> EquipmentManagerComponent;
};
