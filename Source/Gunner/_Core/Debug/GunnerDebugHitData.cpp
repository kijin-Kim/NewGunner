// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerDebugHitData.h"

#include "GameFramework/Character.h"
#include "PhysicsEngine/PhysicsAsset.h"


void FGunnerDebugHitConfirmInfo::CollectDebugHitBoxInfo(USkeletalMeshComponent* MeshComponent)
{
	if (!MeshComponent || !MeshComponent->GetPhysicsAsset())
	{
		return;
	}


	Location = MeshComponent->GetComponentLocation();
	for (const TObjectPtr<USkeletalBodySetup>& BodySetup : MeshComponent->GetPhysicsAsset()->SkeletalBodySetups)
	{
		if (BodySetup)
		{
			FGunnerDebugHitBoxInfo& HitBoxDataEntry = DebugHitBoxInfos.AddDefaulted_GetRef();
			HitBoxDataEntry.BoneWorldTransform = MeshComponent->GetBoneTransform(BodySetup->BoneName);
			HitBoxDataEntry.BoneName = BodySetup->BoneName;
			HitBoxDataEntry.AggGeom = BodySetup->AggGeom;
		}
	}
}
