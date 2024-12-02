// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GunnerEquipmentManagerComponent.generated.h"

USTRUCT()
struct FClientHitScanData
{
	GENERATED_BODY()
	UPROPERTY()
	AActor* HitActor;
	UPROPERTY()
	FVector HitLocation;
	UPROPERTY()
	FName HitBoneName;
	UPROPERTY()
	float TimeStamp;

	UPROPERTY()
	FVector ShooterLocation;
	UPROPERTY()
	FRotator ShooterRotation;
};




class AGunnerEquipment;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquippedEquipmentChangedSignature, AGunnerEquipment*, NewEquippedEquipment, AGunnerEquipment*, OldEquippedEquipment);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UGunnerEquipmentManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGunnerEquipmentManagerComponent();
	void InitEquipmentManagerComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UFUNCTION(BlueprintCallable)
	void AuthAddEquipmentToSlot(int32 SlotIndex, TSubclassOf<AGunnerEquipment> EquipmentClass);
	UFUNCTION(BlueprintCallable)
	void SetCurrentEquipmentByIndex(int32 SlotIndex);
	UFUNCTION(BlueprintCallable)
	AGunnerEquipment* GetEquipmentByIndex(int32 SlotIndex) const;
	UFUNCTION(BlueprintCallable)
	AGunnerEquipment* GetCurrentEquippedEquipment() const;


	UFUNCTION(Server, Reliable)
	void ServerFireHitScan(const TArray<FClientHitScanData>& ClientHitScanData);

	

public:
	UPROPERTY(BlueprintAssignable)
	FOnEquippedEquipmentChangedSignature OnEquippedEquipmentChangedDelegate;

private:
	static void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y);
	void InternalOnShowDebugInfo(AActor* Actor, AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y);
	
	UFUNCTION()
	void OnRep_CurrentEquippedEquipment(AGunnerEquipment* OldEquippedEquipment);
	UFUNCTION()
	void OnRep_EquipmentSlots(const TArray<AGunnerEquipment*>& OldEquipmentSlots);

private:
	const int32 MaxSlots = 3;
	UPROPERTY(ReplicatedUsing = OnRep_CurrentEquippedEquipment)
	TObjectPtr<AGunnerEquipment> CurrentEquippedEquipment;
	UPROPERTY(ReplicatedUsing = OnRep_EquipmentSlots)
	TArray<TObjectPtr<AGunnerEquipment>> EquipmentSlots;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<AGunnerEquipment>> InitialEquipmentClasses;
};
