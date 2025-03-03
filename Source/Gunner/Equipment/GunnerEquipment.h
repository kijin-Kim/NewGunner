// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/Actor.h"
#include "Animation/NexusAnimMontagePlayerInterface.h"
#include "Action/NexusActionDefHandle.h"
#include "GunnerEquipment.generated.h"

class UGunnerEquipmentDef;
class UGunnerLocomotionAnimSet;
class UNexusActionComponent;
class UNexusAction;

UENUM(BlueprintType)
enum class EEquipmentType : uint8
{
	Primary = 0,
	Secondary,
	Melee,
	CSkill,
	QSkill,
	ESkill,
	XSkill,
	EquipmentTypeCount,
};


UCLASS()
class GUNNER_API AGunnerEquipment : public AActor, public INexusAnimMontagePlayerInterface
{
	GENERATED_BODY()

public:
	AGunnerEquipment();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void AttachEquipmentToOwner();
	

	void OnAuthAcquired();
	void OnAuthLost();
	void OnEquipped();
	void OnUnequipped();
	void SetMeshVisibility(bool bVisible);

	virtual void OnRep_Owner() override;

	UFUNCTION(Server, Reliable)
	void ServerAckClientAcquired();


	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UNexusAnimMontagePlayerComponent* GetAnimMontagePlayer();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	USkeletalMeshComponent* GetFirstPersonMeshComponent() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	USkeletalMeshComponent* GetThirdPersonMeshComponent() const;
	EEquipmentType GetEquipmentType() const { return EquipmentType; }

	void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y);

private:
	void AuthAddDesiredActions(const TArray<TSubclassOf<UNexusAction>>& ActionsToAdd, TArray<FNexusActionDefHandle>& AddedActionHandles);
	void AuthRemoveDesiredActions(TArray<FNexusActionDefHandle>& AddedActionHandles);

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
	TObjectPtr<UNexusAnimMontagePlayerComponent> AnimMontagePlayerComponent;
	
	TArray<FNexusActionDefHandle> AddedActionHandlesOnAcquired;
	TArray<FNexusActionDefHandle> AddedActionHandlesOnEquip;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UGunnerLocomotionAnimSet> LocomotionAnimSet;

	UPROPERTY()
	float BulletCount;
	UPROPERTY()
	float MagazineBulletCount;
	UPROPERTY()
	float MaxBulletPerMagazineCount;

public:
	void SetEquipmentDef(UGunnerEquipmentDef* InEquipmentDef);
	UGunnerEquipmentDef* GetEquipmentDef() const { return EquipmentDef; }

private:
	UPROPERTY(Replicated, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGunnerEquipmentDef> EquipmentDef;
};
