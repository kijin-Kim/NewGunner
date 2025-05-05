// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Action/NexusAction.h"
#include "GunnerAction_MeleeAttack.generated.h"

class UGunnerDamageType;
class USphereComponent;
/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerAction_MeleeAttack : public UNexusAction
{
	GENERATED_BODY()

public:
	virtual void OnAddAction() override;
	virtual void OnEndAction() override;
	virtual void OnRemoveAction() override;
	UFUNCTION(BlueprintCallable)
	void StartHitDetection();
	UFUNCTION(BlueprintCallable)
	void StopHitDetection();

private:
	void AuthApplyDamage(AActor* Actor);
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced)
	TObjectPtr<const UGunnerDamageType> DamageType;
	UPROPERTY(EditAnywhere)
	float SphereRadius = 30.0f;
	UPROPERTY()
	TObjectPtr<USphereComponent> SphereComponent;
	UPROPERTY()
	TSet<AActor*> AlreadyHitDetectedActors;
};
