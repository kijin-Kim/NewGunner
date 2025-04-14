// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NexusActionInterface.h"
#include "Animation/NexusAnimMontagePlayerInterface.h"
#include "Cue/NexusCueNetworkProxyInterface.h"
#include "GameFramework/Character.h"
#include "Gunner/Slot/GunnerSlotManagerInterface.h"
#include "Gunner/_Core/GunnerHitBoxInterface.h"
#include "Gunner/_Core/GunnerTeamAgentInterface.h"
#include "Gunner/_Core/GunnerLagCompensationComponent.h"
#include "GunnerCharacter.generated.h"

class UCameraComponent;
class UCameraControllerComponent;
class UNexusAction;
class UNexusActionComponent;
class UGunnerEquipmentManagerComponent;
class USpringArmComponent;
class UWeaponManagerComponent;


UCLASS()
class GUNNER_API AGunnerCharacter
	: public ACharacter,
	  public INexusAnimMontagePlayerInterface,
	  public INexusActionInterface,
	  public IGunnerTeamAgentInterface,
	  public IGunnerHitBoxInterface,
	  public INexusCueNetworkProxyInterface,
	  public IGunnerSlotManagerInterface
{
	GENERATED_BODY()

	friend class UGunnerCheatManagerExtension;

public:
	AGunnerCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void OnConstruction(const FTransform& Transform) override;

	//~ Begin APawn Interface.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End APawn Interface.
	//~ Begin ACharacter Interface.
	virtual void OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState) override;
	virtual bool CanJumpInternal_Implementation() const override;
	//~ End ACharacter Interface.


	//~ Begin IGunnerAnimMontagePlayerInterface Interface.
	virtual UNexusAnimMontagePlayerComponent* GetAnimMontagePlayer_Implementation() override;
	virtual USkeletalMeshComponent* GetFirstPersonMeshComponent_Implementation() const override { return FirstPersonMeshComponent; }
	virtual USkeletalMeshComponent* GetThirdPersonMeshComponent_Implementation() const override { return GetMesh(); }
	//~ End IGunnerAnimMontagePlayerInterface Interface.

	//~ Begin IGunnerActionComponentInterface Interface.
	virtual UNexusActionComponent* GetActionComponent() const override;
	//~ End IGunnerActionComponentInterface Interface.

	

	//~ Begin IGunnerSlotManagerInterface Interface.
	virtual UGunnerSlotManagerComponent* GetSlotManagerComponent() const override;
	//~ End IGunnerSlotManagerInterface Interface.


	//~ Begin IGenericTeamAgentInterface Interface.
	virtual void SetGenericTeamId(const FGenericTeamId& TeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
	virtual FOnGunnerTeamSetSignature* GetOnTeamSetDelegate() override;
	//~ End IGenericTeamAgentInterface Interface.

	//~ Begin IGunnerHitBoxInterface Interface.
	virtual EGunnerHitBoxType GetHitBoxTypeByHitBoneName_Implementation(FName HitBoneName) const override;
	//~ End IGunnerHitBoxInterface Interface.

	//~ Begin INexusCueNetworkProxyInterface Interface.
	UFUNCTION(NetMulticast, Unreliable)
	virtual void NetMulticastTriggerCue(const FNexusTriggerCueParams& CueParams, FNexusLoopingCueHandle CueHandle) override;
	//~ End INexusCueNetworkProxyInterface Interface.

private:
	void OnTeamSetEvent(FGenericTeamId OldTeamID, FGenericTeamId NewTeamID);

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
	TObjectPtr<UNexusAnimMontagePlayerComponent> AnimMontagePlayerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGunnerEquipmentManagerComponent> EquipmentManagerComponent;


	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGunnerLagCompensationComponent> LagCompensationComponent;

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> ThirdPersonMaterialInstances;

	FDelegateHandle OnAddPlayerStateDelegateHandle;
};
