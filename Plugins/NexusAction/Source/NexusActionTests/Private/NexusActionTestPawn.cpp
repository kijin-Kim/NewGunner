#include "NexusActionTestPawn.h"

#include "Action/NexusActionComponent.h"
#include "Action/SubComponent/NexusCueComponent.h"
#include "Action/SubComponent/NexusGameplayTagComponent.h"
#include "Action/SubComponent/NexusPropertyComponent.h"
#include "Action/SubComponent/NexusSideEffectComponent.h"

ANexusActionTestPawn::ANexusActionTestPawn()
{
	ActionComponent = CreateDefaultSubobject<UNexusActionComponent>(TEXT("ActionComponent"));
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
}

UNexusActionComponent* ANexusActionTestPawn::GetActionComponent() const
{
	return ActionComponent;
}

void ANexusActionTestPawn::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();
	if (Controller)
	{
		ActionComponent->SetupActionComponent( this);
	}

}
