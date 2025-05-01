// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerAction_Fire.h"
#include "Gunner/_Core/Debug/GunnerDebugHitData.h"
#include "GunnerAction_DebugFire.generated.h"

/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerAction_DebugFire : public UGunnerAction_Fire
{
	GENERATED_BODY()

public:
	virtual TArray<FHitResult> HitScanTrace() override;
	virtual void AuthHitScanTraceConfirm(const FNexusTargetDataHandle& HitTargetDataHandle) override;
	static void DrawDebugHitBoxData(UWorld* World, const TArray<FGunnerDebugHitBoxDataEntry>& HitBoxData, const FColor& DebugDrawColor, bool bPersistentLines = false, float LifeTime = -1.0f);

protected:
	virtual void AuthOnBeginRewind(TArray<ACharacter*> LagCompensationTargetCharacters, float TimeStamp) override;
	

private:
	UPROPERTY()
	TArray<FGunnerDebugHitConfirmedDataEntry> DebugHitConfirmData;
};
