// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerActionSlotItemBase.h"
#include "GunnerActionGunFire.generated.h"

class UGunnerDamageType;
struct FNexusTargetDataHandle;
/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerActionGunFire : public UGunnerActionSlotItemBase
{
	GENERATED_BODY()

private:
	TArray<FHitResult> FilterDuplicateHitResultsByActor(const TArray<FHitResult>& HitResults);
	TArray<AActor*> GetUniqueActorsFromHitResults(const TArray<FHitResult>& HitResults);
	TArray<AActor*> GetIgnoredActorsByTeam(AActor* EquipmentActorOwner);
	void AuthBeginRewind(TArray<ACharacter*> LagCompensationTargetCharacters, float TimeStamp);
	void AuthEndRewind(TArray<ACharacter*> LagCompensationTargetCharacters);

	void AuthApplyDamageByHitResults(const TArray<FHitResult>& HitResults);
	void AuthApplyDamage(AActor* HitActor, FName HitBoneName, FVector HitNormal);

public:
	UFUNCTION(BlueprintCallable)
	TArray<FHitResult> HitScanTrace();
	UFUNCTION(BlueprintCallable)
	void AuthHitScanTraceConfirm(FNexusTargetDataHandle HitTargetDataHandle);


private:
	UPROPERTY(EditAnywhere, Instanced, Category = "Damage")
	TObjectPtr<const UGunnerDamageType> DamageType;
};
