// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerTestSlotItemPickup.h"

#include "GunnerTestGun.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"


AGunnerTestSlotItemPickup::AGunnerTestSlotItemPickup()
{
	PickupInitialDelay = TNumericLimits<float>::Max();
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
	BoxComponent->SetBoxExtent({50.0f, 50.0f, 50.0f});
}

bool AGunnerTestSlotItemPickup::CanPickup_Implementation(AActor* OtherActor, UActorComponent* OtherComponent) const
{
	bool bResult = Super::CanPickup_Implementation(OtherActor, OtherComponent);
	if (!bResult)
	{
		return false;
	}

	AGunnerTestGun* TestGun = Cast<AGunnerTestGun>(SlotItemInstance);
	return OtherActor && OtherActor != TestGun->LastAgentActor;
}

void AGunnerTestSlotItemPickup::SetPickupEnabled(bool bEnabled)
{
	BoxComponent->SetCollisionResponseToChannel(ECC_Pawn, bEnabled ? ECR_Overlap : ECR_Ignore);
}

UGunnerActionTestDropSlotItem::UGunnerActionTestDropSlotItem()
{
	PickupClass = AGunnerTestSlotItemPickup::StaticClass();
}
