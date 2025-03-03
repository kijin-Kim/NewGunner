// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerActionEquipmentBase.h"
#include "GunnerActionFire.generated.h"

struct FNexusTargetDataHandle;
/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerActionFire : public UGunnerActionEquipmentBase
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
};
