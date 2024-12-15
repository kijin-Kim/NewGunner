// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationDummyActor.h"
#include "LagCompensationHitBoxCapsuleComponent.h"
#include "_Core/HitBox.h"


// Sets default values
ALagCompensationDummyActor::ALagCompensationDummyActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

TArray<FHitBox> ALagCompensationDummyActor::CollectAndGetHitBoxes()
{
	TArray<ULagCompensationHitBoxCapsuleComponent*> CapsuleComponents;
	GetComponents<ULagCompensationHitBoxCapsuleComponent>(CapsuleComponents);
	TArray<FHitBox> HitBoxes;
	for (ULagCompensationHitBoxCapsuleComponent* CapsuleComponent : CapsuleComponents)
	{
		FHitBox HitBox;
		HitBox.Transform = CapsuleComponent->GetComponentTransform();
		HitBox.Radius = CapsuleComponent->GetUnscaledCapsuleRadius();
		HitBox.HalfHeight = CapsuleComponent->GetUnscaledCapsuleHalfHeight();
		HitBox.BoneName = CapsuleComponent->GetBoneName();
		HitBoxes.Add(HitBox);
	}
	return HitBoxes;
}
