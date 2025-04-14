#include "ActionSystemTestPawn.h"

#include "Action/NexusActionComponent.h"

AActionSystemTestPawn::AActionSystemTestPawn()
{
	ActionComponent = CreateDefaultSubobject<UNexusActionComponent>(TEXT("ActionComponent"));
	ActionComponent->SetIsReplicated(true);
}

UNexusActionComponent* AActionSystemTestPawn::GetActionComponent() const
{
	return ActionComponent;
}

void AActionSystemTestPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	ActionComponent->SetupActionComponent(this);
}
