// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PhysicsEngine/AggregateGeom.h"
#include "GunnerDebugHitData.generated.h"


class USkeletalBodySetup;

USTRUCT()
struct FGunnerDebugHitBoxInfo
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FName BoneName;
	UPROPERTY()
	FTransform BoneWorldTransform;
	UPROPERTY()
	FKAggregateGeom AggGeom;
};


USTRUCT()
struct FGunnerDebugHitConfirmInfo
{
	GENERATED_BODY()

public:
	void CollectDebugHitBoxInfo(USkeletalMeshComponent* MeshComponent);

public:
	UPROPERTY()
	TObjectPtr<ACharacter> TargetCharacter;
	
	UPROPERTY()
	bool bServerConfirmedHit = false;
	UPROPERTY()
	float ServerRewindTimeStamp = 0.0f;
	UPROPERTY()
	bool bRewindSnapshotFound = false;

		
	UPROPERTY()
	FVector Location;
	UPROPERTY()
	TArray<FGunnerDebugHitBoxInfo> DebugHitBoxInfos;

};
