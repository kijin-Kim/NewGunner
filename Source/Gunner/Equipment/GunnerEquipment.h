// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/Actor.h"
#include "Gunner/Animation/GunnerAnimMontagePlayerInterface.h"
#include "Gunner/_Core/ActionSystem/GunnerActionDefinitionHandle.h"
#include "GunnerEquipment.generated.h"

class UGunnerLocomotionAnimSet;
class UGunnerActionComponent;
class UGunnerAction;

UENUM(BlueprintType)
enum class EEquipmentType : uint8
{
	Primary,
	Secondary,
	Melee,
	CSkill,
	QSkill,
	ESkill,
	XSkill
};


UCLASS()
class GUNNER_API AGunnerEquipment : public AActor, public IGunnerAnimMontagePlayerInterface
{
	GENERATED_BODY()

public:
	AGunnerEquipment();
	virtual void OnConstruction(const FTransform& Transform) override;
	void AttachEquipmentToOwner();

	void OnAuthAcquired();
	void OnAuthLost();
	void OnEquipped();
	void OnUnequipped();
	void SetMeshVisibility(bool bVisible);

	virtual void OnRep_Owner() override;
	

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UGunnerAnimMontagePlayerComponent* GetAnimMontagePlayer();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	USkeletalMeshComponent* GetFirstPersonMeshComponent() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	USkeletalMeshComponent* GetThirdPersonMeshComponent() const;
	EEquipmentType GetEquipmentType() const { return EquipmentType; }
	FName GetEquipmentName() const { return EquipmentName; }

	void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y);

private:
	void AuthAddDesiredActions(const TArray<TSubclassOf<UGunnerAction>>& ActionsToAdd, TArray<FGunnerActionDefinitionHandle>& AddedActionHandles);
	void AuthRemoveDesiredActions(TArray<FGunnerActionDefinitionHandle>& AddedActionHandles);

	void SetOwnerLocomotionAnimSet(UGunnerLocomotionAnimSet* InLocomotionAnimSet);
	void OnTeamSetEvent(FGenericTeamId OldTeamID, FGenericTeamId NewTeamID);

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	EEquipmentType EquipmentType;


	UPROPERTY()
	TObjectPtr<USceneComponent> DefaultSceneRootComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> FirstPersonMeshComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> ThirdPersonMeshComponent;

	UPROPERTY()
	TObjectPtr<UGunnerAnimMontagePlayerComponent> AnimMontagePlayerComponent;


	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<UGunnerAction>> ActionsToAddOnAcquired;
	TArray<FGunnerActionDefinitionHandle> AddedActionHandlesOnAcquired;
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<UGunnerAction>> ActionsToAddOnEquip;
	TArray<FGunnerActionDefinitionHandle> AddedActionHandlesOnEquip;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UGunnerLocomotionAnimSet> LocomotionAnimSet;

	UPROPERTY(EditAnywhere)
	float BulletCount;
	UPROPERTY(EditAnywhere)
	float MagazineBulletCount;
	UPROPERTY(EditAnywhere)
	float MaxBulletPerMagazineCount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FName EquipmentName;
	
	
	
};
