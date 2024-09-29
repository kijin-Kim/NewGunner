// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gunner/Core/AnimMontagePlayerInterface.h"
#include "Gunner/Core/ActionSystem/GunnerActionDefinitionHandle.h"
#include "GunnerEquipment.generated.h"

class UGunnerLocomotionAnimSet;
class UGunnerActionComponent;
class UGunnerAction;

UCLASS()
class GUNNER_API AGunnerEquipment : public AActor, public IAnimMontagePlayerInterface
{
	GENERATED_BODY()

public:
	AGunnerEquipment();
	virtual void SetOwner(AActor* NewOwner) override;
	UFUNCTION(BlueprintCallable)
	void AddActionsOnEquip();
	UFUNCTION(BlueprintCallable)
	void RemoveActionsOnEquip();
	void AddActionsOnAcquire();
	void RemoveActionsOnAcquire(AActor* OldOwner);
	void AttachEquipmentToOwner();
	void OnEquipped();
	void OnUnequipped();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UAnimMontagePlayerComponent* GetAnimMontagePlayer();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	USkeletalMeshComponent* GetFirstPersonMeshComponent() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	USkeletalMeshComponent* GetThirdPersonMeshComponent() const;

private:
	void SetOwnerLocomotionAnimSet(UGunnerLocomotionAnimSet* InLocomotionAnimSet);
	

private:
	UPROPERTY()
	TObjectPtr<USceneComponent> DefaultSceneRootComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> FirstPersonMeshComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> ThirdPersonMeshComponent;
	
	UPROPERTY()
	TObjectPtr<UAnimMontagePlayerComponent> AnimMontagePlayerComponent;


	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<UGunnerAction>> ActionsToAddOnAcquire;
	TArray<FGunnerActionDefinitionHandle> AddedActionHandlesOnAcquire;
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<UGunnerAction>> ActionsToAddOnEquip;
	TArray<FGunnerActionDefinitionHandle> AddedActionHandlesOnEquip;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UGunnerLocomotionAnimSet> LocomotionAnimSet;
	
};
