// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Action/NexusAction.h"
#include "Gunner/_Core/GunnerBlueprintFunctionLibrary.h"
#include "Gunner/_Core/Debug/GunnerDebugHitData.h"


#include "GunnerAction_Fire.generated.h"

class USkeletalBodySetup;
class UGunnerDamageType;
struct FNexusTargetDataHandle;

/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerAction_Fire : public UNexusAction
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	TArray<FHitResult> HitScanTrace();
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void AuthHitScanTraceConfirm(const FNexusTargetDataHandle& HitTargetDataHandle);
	static void DrawDebugHitBoxData(UWorld* World, const TArray<FGunnerDebugHitBoxInfo>& HitBoxData, const FColor& DebugDrawColor, bool bPersistentLines = false, float LifeTime = -1.0f);

protected:
	void AuthOnBeginRewind(TArray<ACharacter*> LagCompensationTargetCharacters, float TargetTimeStamp);
	void AuthOnEndRewind(TArray<ACharacter*> LagCompensationTargetCharacters, const TArray<FHitResult>& HitResults);
	TArray<AActor*> GetUniqueActorsFromHitResults(const TArray<FHitResult>& HitResults);
	TArray<AActor*> GetIgnoredActorsByTeam(APlayerState* PlayerState);
	
private:
	void AuthApplyDamageByHitResults(const TArray<FHitResult>& HitResults);
	void AuthApplyDamage(AActor* HitActor, FName HitBoneName, FVector HitNormal, const FHitResult& HitResult);
	void DrawDebugHitScanTrace(const TArray<FHitResult>& HitResults);
	void CalculateTraceStartEnd( FVector& OutTraceStart, FVector& OutTraceEnd) const;



protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced)
	TObjectPtr<const UGunnerDamageType> DamageType;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bEnableLagCompensation = true;
	

	UPROPERTY(EditAnywhere)
	bool bEnableDebug = false;
	UPROPERTY()
	TArray<FGunnerDebugHitConfirmInfo> DebugHitConfirmInfos;
};
