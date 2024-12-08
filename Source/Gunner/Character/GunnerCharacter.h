// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"
#include "Gunner/Animation/GunnerAnimMontagePlayerInterface.h"
#include "Gunner/_Core/ActionSystem/GunnerActionComponentInterface.h"
#include "Gunner/_Core/Event/GunnerEventManagerInterface.h"
#include "GunnerCharacter.generated.h"

class UCameraComponent;
class UCameraControllerComponent;
class UGunnerAction;
class UGunnerActionComponent;
class UGunnerEquipmentManagerComponent;
class USpringArmComponent;
class UWeaponManagerComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGunnerCharacterPlayerStateChanged, APlayerState* OldPlayerState, APlayerState* NewPlayerState);

UCLASS()
class GUNNER_API AGunnerCharacter : public ACharacter, public IGunnerAnimMontagePlayerInterface, public IGunnerActionComponentInterface, public IGunnerEventManagerInterface
{
	GENERATED_BODY()

	friend class UGunnerCheatManagerExtension;

public:
	AGunnerCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	//~ Begin APawn Interface.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End APawn Interface.
	
	//~ Begin ACharacter Interface.
	virtual void OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState) override;
	virtual bool CanJumpInternal_Implementation() const override;
	//~ End ACharacter Interface.

	

	//~ Begin IGunnerAnimMontagePlayerInterface Interface.
	virtual UGunnerAnimMontagePlayerComponent* GetAnimMontagePlayer_Implementation() override;
	virtual USkeletalMeshComponent* GetFirstPersonMeshComponent_Implementation() const override { return FirstPersonMeshComponent; }
	virtual USkeletalMeshComponent* GetThirdPersonMeshComponent_Implementation() const override { return GetMesh(); }
	//~ End IGunnerAnimMontagePlayerInterface Interface.

	//~ Begin IGunnerActionComponentInterface Interface.
	virtual UGunnerActionComponent* GetActionComponent() const override;
	//~ End IGunnerActionComponentInterface Interface.

	//~ Begin IGunnerEventManagerInterface Interface.
	virtual UGunnerEventManagerComponent* GetEventManagerComponent() const override;
	//~ End IGunnerEventManagerInterface Interface.


public:
	FOnGunnerCharacterPlayerStateChanged OnPlayerStateChangedDelegate;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> FirstPersonMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> FirstPersonSpringArmComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;
	UPROPERTY()
	TObjectPtr<UCameraControllerComponent> CameraControllerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGunnerAnimMontagePlayerComponent> AnimMontagePlayerComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGunnerEquipmentManagerComponent> EquipmentManagerComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<UGunnerAction>> InitialActions;
	UPROPERTY(EditAnywhere)
	TMap<FGameplayTag, float> PropertiesToAddOnSpawn;
};
