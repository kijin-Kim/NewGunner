// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerPickup.h"
#include "GunnerEquipmentPickup.generated.h"

class AGunnerEquipment;

UCLASS()
class GUNNER_API AGunnerEquipmentPickup : public AGunnerPickup
{
	GENERATED_BODY()

public:
	AGunnerEquipmentPickup();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual bool CanPickup_Implementation(AActor* OtherActor, UActorComponent* OtherComponent) const override;
	virtual void OnPickup_Implementation(AActor* OtherActor, UActorComponent* OtherComponent) override;
	
private:
	void CopyMeshFromSource();
	
	UFUNCTION()
	void OnRep_EquipmentInstance();
	
private:
	UPROPERTY(ReplicatedUsing=OnRep_EquipmentInstance, EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", ExposeOnSpawn = "true"))
	AGunnerEquipment* EquipmentInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* PickupMeshComponent;
};
