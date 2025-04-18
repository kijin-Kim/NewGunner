#include "GunnerTestPawn.h"

#include "GunnerTestActionComponent.h"
#include "Action/NexusActionComponent.h"
#include "Action/SubComponent/NexusCueComponent.h"
#include "Action/SubComponent/NexusGameplayTagComponent.h"
#include "Action/SubComponent/NexusPropertyComponent.h"
#include "Action/SubComponent/NexusSideEffectComponent.h"
#include "Gunner/Slot/GunnerSlotManagerComponent.h"


AGunnerTestSlotItemPickup::AGunnerTestSlotItemPickup()
{
	PickupInitialDelay = TNumericLimits<float>::Max();
}

UGunnerActionTestDropSlotItem::UGunnerActionTestDropSlotItem()
{
	PickupClass = AGunnerTestSlotItemPickup::StaticClass();
}

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


	SlotManagerComponent = CreateDefaultSubobject<UGunnerSlotManagerComponent>(TEXT("SlotManagerComponent"));
	SlotManagerComponent->SetIsReplicated(true);
	SlotManagerComponent->DropSlotItemActionClass = UGunnerActionTestDropSlotItem::StaticClass();
}

UNexusActionComponent* AGunnerTestPawn::GetActionComponent() const
{
	return ActionComponent;
}

UGunnerSlotManagerComponent* AGunnerTestPawn::GetSlotManagerComponent() const
{
	return SlotManagerComponent;
}

void AGunnerTestPawn::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();
	if (Controller)
	{
		ActionComponent->SetupActionComponent( this);
	}

}
