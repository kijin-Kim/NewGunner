// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GunnerEquipmentManagerComponent.generated.h"


class AGunnerEquipment;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UGunnerEquipmentManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGunnerEquipmentManagerComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UFUNCTION(BlueprintCallable)
	void AddEquipmentToSlot(int32 SlotIndex, TSubclassOf<AGunnerEquipment> EquipmentClass);
	UFUNCTION(BlueprintCallable)
	void SetCurrentEquipmentByIndex(int32 SlotIndex);
	UFUNCTION(BlueprintCallable)
	AGunnerEquipment* GetEquipmentByIndex(int32 SlotIndex) const;
	UFUNCTION(BlueprintCallable)
	AGunnerEquipment* GetCurrentEquipment() const;

private:
	UFUNCTION()
	void OnRep_CurrentEquipment(AGunnerEquipment* LastEquipment);

private:
	const int32 MaxSlots = 3;
	UPROPERTY(ReplicatedUsing = OnRep_CurrentEquipment)
	TObjectPtr<AGunnerEquipment> CurrentEquipment;
	UPROPERTY(Replicated)
	TArray<TObjectPtr<AGunnerEquipment>> EquipmentSlots;
	
};
