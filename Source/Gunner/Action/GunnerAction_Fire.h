// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerAction_SlotItemBase.h"
#include "Gunner/_Core/GunnerBlueprintFunctionLibrary.h"


#include "GunnerAction_Fire.generated.h"

class USkeletalBodySetup;
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
	virtual TArray<FHitResult> HitScanTrace();
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	virtual void AuthHitScanTraceConfirm(const FNexusTargetDataHandle& HitTargetDataHandle);

protected:
	virtual void AuthOnBeginRewind(TArray<ACharacter*> LagCompensationTargetCharacters, float TimeStamp);
	virtual void AuthOnEndRewind(TArray<ACharacter*> LagCompensationTargetCharacters);
	
	TArray<FHitResult> FilterDuplicateHitResultsByActor(const TArray<FHitResult>& HitResults);
	TArray<AActor*> GetUniqueActorsFromHitResults(const TArray<FHitResult>& HitResults);
	TArray<AActor*> GetIgnoredActorsByTeam(APlayerState* PlayerState);
	
private:
	
	void AuthApplyDamageByHitResults(const TArray<FHitResult>& HitResults);
	void AuthApplyDamage(AActor* HitActor, FName HitBoneName, FVector HitNormal);

private:


protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced)
	TObjectPtr<const UGunnerDamageType> DamageType;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bEnableLagCompensation = true;
};
