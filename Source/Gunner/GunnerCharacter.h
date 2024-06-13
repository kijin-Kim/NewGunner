// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Core/AnimMontagePlayerInterface.h"
#include "GameFramework/Character.h"
#include "GunnerCharacter.generated.h"

class UAnimMontagePlayerComponent;
class UHealthComponent;
class USpringArmComponent;
class UWeaponManagerComponent;
class UInputAction;
class UInputMappingContext;
class UCameraComponent;



UCLASS()
class GUNNER_API AGunnerCharacter : public ACharacter, public IAnimMontagePlayerInterface
{
	GENERATED_BODY()

	friend class UGunnerCheatManagerExtension;
public:
	AGunnerCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Controller() override;
	virtual bool CanJumpInternal_Implementation() const override;

	virtual UAnimMontagePlayerComponent* GetAnimMontagePlayer_Implementation() override;
	virtual USkeletalMeshComponent* GetFirstPersonMeshComponent_Implementation() const override { return FirstPersonMeshComponent; }
	virtual USkeletalMeshComponent* GetThirdPersonMeshComponent_Implementation() const override { return GetMesh(); }
	bool IsRunning() const;

	

private:
	void SetRunning(bool bNewRunning);
	UFUNCTION(Server, Reliable)
	void ServerRun(bool bNewRunning);


	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void SetupMappingContext();

public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> MoveForwardRightAction;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> MoveBackwardLeftAction;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> JumpAction;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> LookAction;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> CrouchAction;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> WalkAction;
	
	UPROPERTY(EditDefaultsOnly)
	float BaseTurnRate = 45.0f;
	UPROPERTY(EditDefaultsOnly)
	float MouseSensitivity = 1.0f;
	
	

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
	TObjectPtr<UAnimMontagePlayerComponent> AnimMontagePlayerComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UHealthComponent> HealthComponent;
	UPROPERTY(BlueprintReadOnly)
	bool bIsRunning = true;
};
