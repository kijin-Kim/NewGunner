// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerAction_SlotItemBase.h"
#include "GunnerAction_Fire.generated.h"

class UGunnerDamageType;
struct FNexusTargetDataHandle;
/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerAction_Fire : public UGunnerAction_SlotItemBase
{
	GENERATED_BODY()


public:
	UFUNCTION(BlueprintCallable)
	TArray<FHitResult> HitScanTrace();
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void AuthHitScanTraceConfirm(const FNexusTargetDataHandle& HitTargetDataHandle);

private:
	TArray<FHitResult> FilterDuplicateHitResultsByActor(const TArray<FHitResult>& HitResults);
	TArray<AActor*> GetUniqueActorsFromHitResults(const TArray<FHitResult>& HitResults);
	TArray<AActor*> GetIgnoredActorsByTeam(APlayerState* PlayerState);
	void AuthBeginRewind(TArray<ACharacter*> LagCompensationTargetCharacters, float TimeStamp);
	void AuthEndRewind(TArray<ACharacter*> LagCompensationTargetCharacters);

	void AuthApplyDamageByHitResults(const TArray<FHitResult>& HitResults);
	void AuthApplyDamage(AActor* HitActor, FName HitBoneName, FVector HitNormal);




protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced)
	TObjectPtr<const UGunnerDamageType> DamageType;
};
