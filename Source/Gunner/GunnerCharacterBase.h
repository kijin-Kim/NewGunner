// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "GunnerCharacterBase.generated.h"

class UInputAction;
class UInputMappingContext;
class UCameraComponent;

UCLASS()
class GUNNER_API AGunnerCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	AGunnerCharacterBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;
	
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Controller() override;

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
	
	UPROPERTY(EditDefaultsOnly)
	float BaseTurnRate = 30.0f;
	UPROPERTY(EditDefaultsOnly)
	float MouseSensitivity = 1.0f;
	UPROPERTY(EditDefaultsOnly)
	float WalkSpeedMultiplier = 0.6f;
	float MaxWalkSpeedCache = 0.0f;
	

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> FirstPersonMeshComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;
};
