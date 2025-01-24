// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerEquipment.h"
#include "Components/ActorComponent.h"
#include "GunnerEquipmentManagerComponent.generated.h"


class AGunnerEquipment;


USTRUCT(BlueprintType)
struct FEquipmentSlot
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EEquipmentType DesiredEquipmentType;
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AGunnerEquipment> SlottedEquipment;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquippedEquipmentChangedSignature, AGunnerEquipment*, NewEquippedEquipment, AGunnerEquipment*, OldEquippedEquipment);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UGunnerEquipmentManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGunnerEquipmentManagerComponent();
	void InitEquipmentManagerComponent();
	void RelaseEquipmentManagerComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UFUNCTION(BlueprintCallable)
	void AuthAddEquipmentToSlotByClass(TSubclassOf<AGunnerEquipment> EquipmentClass);
	UFUNCTION(BlueprintCallable)
	void AuthAddEquipment(AGunnerEquipment* NewEquipment);
	void AuthRemoveAllEquipments();


	UFUNCTION(BlueprintCallable)
	AGunnerEquipment* DropCurrentEquipment();
	UFUNCTION(BlueprintCallable)
	void SetCurrentEquipmentByEquipmentType(EEquipmentType EquipmentType);
	UFUNCTION(BlueprintCallable)
	AGunnerEquipment* GetEquipmentByEquipmentType(EEquipmentType EquipmentType) const;

	UFUNCTION(BlueprintCallable)
	AGunnerEquipment* GetCurrentEquippedEquipment() const;

public:
	UPROPERTY(BlueprintAssignable)
	FOnEquippedEquipmentChangedSignature OnEquippedEquipmentChangedDelegate;

private:
	static void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y);
	void InternalOnShowDebugInfo(AActor* Actor, AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y);

	UFUNCTION()
	void OnRep_CurrentEquippedEquipment(AGunnerEquipment* OldEquippedEquipment);


private:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentEquippedEquipment)
	TObjectPtr<AGunnerEquipment> CurrentEquippedEquipment;
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<FEquipmentSlot> EquipmentSlots;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<AGunnerEquipment>> InitialEquipmentClasses;

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UAnimInstance> SnapshotAnimInstanceClass;
};
