// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerItemPickup_Test.h"

#include "GunnerItem_Test.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"


AGunnerItemPickup_Test::AGunnerItemPickup_Test()
{
	PickupInitialDelay = TNumericLimits<float>::Max();
	BoxComponent->SetBoxExtent({50.0f, 50.0f, 50.0f});
}

bool AGunnerItemPickup_Test::CanPickup_Implementation(AActor* OtherActor, UActorComponent* OtherComponent) const
{
	bool bResult = Super::CanPickup_Implementation(OtherActor, OtherComponent);
	if (!bResult)
	{
		return false;
	}

	AGunnerItem_Test* TestItem = Cast<AGunnerItem_Test>(SlotItem);
	return OtherActor && OtherActor != TestItem->LastAgentActor;
}

void AGunnerItemPickup_Test::SetPickupEnabled(bool bEnabled)
{
	BoxComponent->SetCollisionResponseToChannel(ECC_Pawn, bEnabled ? ECR_Overlap : ECR_Ignore);
}

UGunnerActionTestDropSlotItem::UGunnerActionTestDropSlotItem()
{
	PickupClass = AGunnerItemPickup_Test::StaticClass();
}
