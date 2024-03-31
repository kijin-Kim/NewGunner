// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "GunnerCharacter.generated.h"

class USpringArmComponent;
class UWeaponManagerComponent;
class UInputAction;
class UInputMappingContext;
class UCameraComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFirePressedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFireReleasedSignature);



UCLASS()
class GUNNER_API AGunnerCharacter : public ACharacter
{
	GENERATED_BODY()

	friend class UGunnerCheatManagerExtension;
public:
	AGunnerCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	void OnFirePressed();
	void OnFireReleased();
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Controller() override;

	USkeletalMeshComponent* GetFirstPersonMeshComponent() const { return FirstPersonMeshComponent; }

protected:
	virtual bool CanJumpInternal_Implementation() const override;

private:
	void Walk();
	void LocalWalk();
	UFUNCTION(Server, Reliable)
	void ServerWalk();
	void Run();
	void LocalRun();
	UFUNCTION(Server, Reliable)
	void ServerRun();


	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void SetupMappingContext();

public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> JumpAction;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> LookAction;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> CrouchAction;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> WalkAction;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> FireAction;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> AltFireAction;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> PrimaryWeaponEquipAction;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> SecondaryWeaponEquipAction;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> MeleeWeaponEquipAction;


	UPROPERTY(EditDefaultsOnly)
	float BaseTurnRate = 45.0f;
	UPROPERTY(EditDefaultsOnly)
	float MouseSensitivity = 1.0f;
	UPROPERTY(EditDefaultsOnly)
	float WalkSpeedMultiplier = 0.6f;
	float MaxWalkSpeedCache = 0.0f;


	UPROPERTY(BlueprintAssignable)
	FOnFirePressedSignature OnFirePressedDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnFirePressedSignature OnFireReleasedDelegate;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<USkeletalMeshComponent> FirstPersonMeshComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> FirstPersonSpringArmComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UWeaponManagerComponent> WeaponManagerComponent;
};
