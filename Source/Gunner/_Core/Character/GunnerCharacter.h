// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NexusActionInterface.h"
#include "Action/NexusActionDefHandle.h"
#include "Action/SubComponent/NexusEventManagerComponent.h"
#include "Animation/NexusAnimMontagePlayerInterface.h"
#include "Cue/NexusCueNetworkProxyInterface.h"
#include "Event/NexusEventMessage.h"
#include "GameFramework/Character.h"
#include "Gunner/Item/GunnerInventoryManagerInterface.h"
#include "Gunner/_Core/GunnerHitBoxInterface.h"
#include "Gunner/_Core/GunnerTeamAgentInterface.h"
#include "Gunner/_Core/GunnerLagCompensationComponent.h"
#include "GunnerCharacter.generated.h"

class AGunnerItem;
class UGunnerActionSet;
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
	  public IGunnerInventoryManagerInterface
{
	GENERATED_BODY()

	friend class UGunnerCheatManagerExtension;

public:
	AGunnerCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void PostInitializeComponents() override;
	virtual void UnPossessed() override;

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


	//~ Begin IGunnerInventoryInterface Interface.
	virtual UGunnerInventoryManagerComponent* GetInventoryManagerComponent() const override;
	//~ End IGunnerInventoryInterface Interface.


	//~ Begin IGenericTeamAgentInterface Interface.
	virtual void SetGenericTeamId(const FGenericTeamId& TeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
	virtual FOnGunnerTeamSetSignature* GetOnTeamSetDelegate() override;
	//~ End IGenericTeamAgentInterface Interface.

	//~ Begin IGunnerHitBoxInterface Interface.
	virtual EGunnerHitPartType GetHitPartTypeByHitBoneName_Implementation(FName HitBoneName) const override;
	//~ End IGunnerHitBoxInterface Interface.

	//~ Begin INexusCueNetworkProxyInterface Interface.
	UFUNCTION(NetMulticast, Unreliable)
	virtual void NetMulticastTriggerCue(TSubclassOf<ANexusCue> CueClass, FNexusPredictionTag PredictionTag, const FNexusCueParameters& CueParameters) override;
	//~ End INexusCueNetworkProxyInterface Interface.


	void AuthAddActionSets();
	void AuthRemoveActionSets();

private:
	void OnTeamSetEvent(FGenericTeamId OldTeamID, FGenericTeamId NewTeamID);

	UFUNCTION()
	void OnTagAdded(const FGameplayTag& Tag);
	UFUNCTION()
	void OnTagRemoved(const FGameplayTag& Tag);

	void CreateOrShowCharacterWidgets(APlayerController* PC);

protected:
	UPROPERTY(EditAnywhere)
	TArray<const UGunnerActionSet*> ActionSets;
	TArray<FNexusActionDefHandle> AddedActionHandles;
	UPROPERTY()
	TArray<AGunnerItem*> AddedItems;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<UUserWidget>> CharacterWidgetClasses;

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


	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGunnerLagCompensationComponent> LagCompensationComponent;

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> ThirdPersonMaterialInstances;
	
	UPROPERTY()
	TMap<TSubclassOf<UUserWidget>, TObjectPtr<UUserWidget>> CharacterWidgets;
};
