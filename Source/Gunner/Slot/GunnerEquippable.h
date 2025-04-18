// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerSlotManagerComponent.h"
#include "Animation/NexusAnimMontagePlayerInterface.h"
#include "GunnerEquippable.generated.h"

class UGunnerLocomotionAnimSet;

UCLASS()
class GUNNER_API AGunnerEquippable : public AGunnerSlotItem, public INexusAnimMontagePlayerInterface
{
	GENERATED_BODY()

public:
	AGunnerEquippable();
	virtual void OnConstruction(const FTransform& Transform) override;

	//~ Begin INexusAnimMontagePlayerInterface
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UNexusAnimMontagePlayerComponent* GetAnimMontagePlayer();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	USkeletalMeshComponent* GetFirstPersonMeshComponent() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	USkeletalMeshComponent* GetThirdPersonMeshComponent() const;
	//~ End INexusAnimMontagePlayerInterface
	
	virtual void OnAcquired(AActor* AgentActor) override;
	virtual void OnRemoved(AActor* AgentActor) override;
	
	virtual void OnActivated(AActor* AgentActor) override;
	virtual void OnDeactivated(AActor* AgentActor) override;

private:
	void AttachToExpliciteOwner(AActor* AgentActor) const;
	void SetMeshVisibility(AActor* AgentActor, bool bVisible) const;
	void SetAgentActorLocomotionAnimSet(AActor* AgentActor, UGunnerLocomotionAnimSet* InLocomotionAnimSet) const;
	void SetRenderCustomDepth(bool bSetRenderCustomDepth);
	void SetCustomDepthStencilValue(int32 StencilValue) const;

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
