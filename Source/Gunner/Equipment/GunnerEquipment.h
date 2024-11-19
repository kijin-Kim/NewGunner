// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gunner/_Core/ActionSystem/GunnerActionProperty.h"
#include "Gunner/Animation/GunnerAnimMontagePlayerInterface.h"
#include "Gunner/_Core/ActionSystem/GunnerActionDefinitionHandle.h"
#include "GunnerEquipment.generated.h"

class UGunnerLocomotionAnimSet;
class UGunnerActionComponent;
class UGunnerAction;

UCLASS()
class GUNNER_API AGunnerEquipment : public AActor, public IGunnerAnimMontagePlayerInterface
{
	GENERATED_BODY()

public:
	AGunnerEquipment();

	void AttachEquipmentToOwner();
	
	void OnAcquired();
	void OnLost();
	void OnEquipped();
	void OnUnequipped();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UGunnerAnimMontagePlayerComponent* GetAnimMontagePlayer();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	USkeletalMeshComponent* GetFirstPersonMeshComponent() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	USkeletalMeshComponent* GetThirdPersonMeshComponent() const;
	
	void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y);

private:
	void AuthAddDesiredActions(const TArray<TSubclassOf<UGunnerAction>>& ActionsToAdd, TArray<FGunnerActionDefinitionHandle>& AddedActionHandles);
	void AuthRemoveDesiredActions(TArray<FGunnerActionDefinitionHandle>& AddedActionHandles);
	
	void SetOwnerLocomotionAnimSet(UGunnerLocomotionAnimSet* InLocomotionAnimSet);
	
	void SetMeshVisibility(bool bVisible);


private:
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
	TMap<FGameplayTag, float> PropertiesToAddOnEquip;
};
