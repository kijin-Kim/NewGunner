// Copyright Epic Games, Inc. All Rights Reserved.

#include "PickupComponent.h"

UPickupComponent::UPickupComponent()
{
	SphereRadius = 32.f;
}

void UPickupComponent::BeginPlay()
{
	Super::BeginPlay();
	const AActor* ActorOwner = GetOwner();
	if (ActorOwner && ActorOwner->HasAuthority())
	{
		OnComponentBeginOverlap.AddDynamic(this, &UPickupComponent::OnSphereBeginOverlap);
	}
}

void UPickupComponent::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AGunnerCharacterBase* Character = Cast<AGunnerCharacterBase>(OtherActor);
	if (Character != nullptr)
	{
		OnPickUp.Broadcast(Character);
		OnComponentBeginOverlap.RemoveAll(this);
	}
}
