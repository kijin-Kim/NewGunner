#include "GunnerTestPawn.h"

#include "GunnerTestActionComponent.h"
#include "GunnerTestGun.h"
#include "GunnerTestSlotItemPickup.h"
#include "Action/NexusActionComponent.h"
#include "Action/SubComponent/NexusCueComponent.h"
#include "Action/SubComponent/NexusGameplayTagComponent.h"
#include "Action/SubComponent/NexusPropertyComponent.h"
#include "Action/SubComponent/NexusSideEffectComponent.h"
#include "Components/SphereComponent.h"
#include "Gunner/Slot/GunnerInventoryManagerComponent.h"


AGunnerTestPawn::AGunnerTestPawn()
{
	ActionComponent = CreateDefaultSubobject<UGunnerTestActionComponent>(TEXT("ActionComponent"));
	ActionComponent->SetIsReplicated(true);
	CueComponent = CreateDefaultSubobject<UNexusCueComponent>(TEXT("CueComponent"));
	CueComponent->SetIsReplicated(true);
	PropertyComponent = CreateDefaultSubobject<UNexusPropertyComponent>(TEXT("PropertyComponent"));
	PropertyComponent->SetIsReplicated(true);
	GameplayTagComponent = CreateDefaultSubobject<UNexusGameplayTagComponent>(TEXT("GameplayTagComponent"));
	GameplayTagComponent->SetIsReplicated(true);
	SideEffectComponent = CreateDefaultSubobject<UNexusSideEffectComponent>(TEXT("SideEffectComponent"));
	SideEffectComponent->SetIsReplicated(true);
	PredictionComponent = CreateDefaultSubobject<UNexusPredictionComponent>(TEXT("PredictionComponent"));
	PredictionComponent->SetIsReplicated(true);
	EventManagerComponent = CreateDefaultSubobject<UNexusEventManagerComponent>(TEXT("EventManagerComponent"));
	EventManagerComponent->SetIsReplicated(true);


	SlotManagerComponent = CreateDefaultSubobject<UGunnerInventoryManagerComponent>(TEXT("SlotManagerComponent"));
	SlotManagerComponent->SetIsReplicated(true);
	SlotManagerComponent->DropSlotItemActionClass = UGunnerActionTestDropSlotItem::StaticClass();

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionObjectType(ECC_Pawn);
	SphereComponent->SetSphereRadius(100.0f);
	SetRootComponent(SphereComponent);
}

UNexusActionComponent* AGunnerTestPawn::GetActionComponent() const
{
	return ActionComponent;
}

UGunnerInventoryManagerComponent* AGunnerTestPawn::GetInventoryManagerComponent() const
{
	return SlotManagerComponent;
}

void AGunnerTestPawn::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();
	if (Controller)
	{
		ActionComponent->SetupActionComponent(this);
	}
}
