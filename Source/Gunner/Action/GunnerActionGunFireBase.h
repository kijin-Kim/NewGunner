// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gunner/_Core/ReplicatableObject.h"
#include "Gunner/_Core/ActionSystem/GunnerAction.h"
#include "GunnerActionGunFireBase.generated.h"


UCLASS(BlueprintType)
class UHitScanSignDataObject : public UReplicatableObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	AActor* AgentActor;
	UPROPERTY(BlueprintReadOnly)
	TArray<FHitResult> LocalHitResult;
};

/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerActionGunFireBase : public UGunnerAction
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void FireHitScan(TArray<FHitResult>& OutHitResults);
	UFUNCTION(BlueprintCallable)
	UHitScanSignDataObject* MakeHitScanSignDataObject(AActor* AgentActor, const TArray<FHitResult>& HitResults);

};
