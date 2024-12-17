// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GunnerEquipmentManagerComponent.generated.h"


struct FHitBox;
class AGunnerEquipment;

UCLASS(BlueprintType)
class GUNNER_API UGunnerHitMessageData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	AGunnerEquipment* HitEquipment;
	UPROPERTY(BlueprintReadOnly)
	FName HitBoneName;
	UPROPERTY(BlueprintReadOnly)
	FVector HitNormal;
};


USTRUCT()
struct FClientHitScanData
{
	GENERATED_BODY()

	FClientHitScanData() = default;

	FClientHitScanData(const FHitResult& InHitResult)
		: HitActor(InHitResult.GetActor())
		  , HitLocation(InHitResult.ImpactPoint)
		  , HitBoneName(InHitResult.BoneName)
	{
	}


	UPROPERTY()
	AActor* HitActor;
	UPROPERTY()
	FVector HitLocation;
	UPROPERTY()
	FName HitBoneName;
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
	void RelaseEquipmentManagerComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UFUNCTION(BlueprintCallable)
	void AuthAddEquipmentToSlot(int32 SlotIndex, TSubclassOf<AGunnerEquipment> EquipmentClass);
	void AuthRemoveAllEquipments();
	UFUNCTION(BlueprintCallable)
	void SetCurrentEquipmentByIndex(int32 SlotIndex);
	UFUNCTION(BlueprintCallable)
	AGunnerEquipment* GetEquipmentByIndex(int32 SlotIndex) const;
	UFUNCTION(BlueprintCallable)
	AGunnerEquipment* GetCurrentEquippedEquipment() const;


	UFUNCTION(Server, Reliable)
	void ServerRequestHitScanConfirm(const TArray<FClientHitScanData>& ClientHitScanData, float TimeStamp);
	void LocalHitScan(TArray<FHitResult>& OutHitResults, const TArray<AActor*>& ActorsToIgnore = {});
	void LocalHitScan2(TArray<FHitResult>& OutHitResults, const FCollisionQueryParams& CollisionQueryParams);
	void AuthApplyDamage(AActor* HitActor, FName BoneName, FVector HitNormal);

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


	void DrawDebugHitBoxByHitResult(const TArray<FHitResult>& HitResults);
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticastSendCurrentHitBoxes(const TArray<FHitBox>& HitBoxes, FName HitBoneName, FVector HitLocation, FColor HitColor, FColor NonHitColor);

private:
	const int32 MaxSlots = 3;
	UPROPERTY(ReplicatedUsing = OnRep_CurrentEquippedEquipment)
	TObjectPtr<AGunnerEquipment> CurrentEquippedEquipment;
	UPROPERTY(ReplicatedUsing = OnRep_EquipmentSlots)
	TArray<TObjectPtr<AGunnerEquipment>> EquipmentSlots;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<AGunnerEquipment>> InitialEquipmentClasses;


public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UAnimInstance> SnapshotAnimInstanceClass;

private:
	void Draw(ACharacter* Character, FColor Color);
};
