// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerInventoryManagerComponent.h"
#include "GunnerSlotItem.h"
#include "Animation/NexusAnimMontagePlayerInterface.h"
#include "GunnerEquipmentItem.generated.h"

class UGunnerLocomotionAnimSet;

UCLASS()
class GUNNER_API AGunnerEquipmentItem : public AGunnerSlotItem, public INexusAnimMontagePlayerInterface
{
	GENERATED_BODY()

public:
	AGunnerEquipmentItem();
	virtual void OnConstruction(const FTransform& Transform) override;

	//~ Begin INexusAnimMontagePlayerInterface
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UNexusAnimMontagePlayerComponent* GetAnimMontagePlayer();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	USkeletalMeshComponent* GetFirstPersonMeshComponent() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	USkeletalMeshComponent* GetThirdPersonMeshComponent() const;
	//~ End INexusAnimMontagePlayerInterface
	
	virtual void OnAcquired(AActor* InAgentActor) override;
	virtual void OnRemoved() override;
	
	virtual void OnActivated() override;
	virtual void OnDeactivated() override;



private:
	UFUNCTION(BlueprintCallable)
	void AttachToAgentActor() const;
	UFUNCTION(BlueprintCallable)
	void SetMeshVisibility(bool bVisible) const;
	UFUNCTION(BlueprintCallable)
	void SetAgentActorLocomotionAnimSet(UGunnerLocomotionAnimSet* InLocomotionAnimSet) const;
	UFUNCTION(BlueprintCallable)
	void SetRenderCustomDepth(bool bSetRenderCustomDepth);
	UFUNCTION(BlueprintCallable)
	void SetCustomDepthStencilValue(int32 StencilValue) const;

protected:
	UPROPERTY(EditAnywhere)
	FName AttachSocketName = TEXT("Socket_MasterWeapon");

private:
	UPROPERTY()
	TObjectPtr<USceneComponent> DefaultSceneRootComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> FirstPersonMeshComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> ThirdPersonMeshComponent;

	UPROPERTY()
	TObjectPtr<UNexusAnimMontagePlayerComponent> AnimMontagePlayerComponent;


	UPROPERTY(EditAnywhere)
	TObjectPtr<UGunnerLocomotionAnimSet> LocomotionAnimSet;


	
};
