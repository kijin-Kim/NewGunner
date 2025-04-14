#pragma once

#include "CoreMinimal.h"
#include "NexusActionInterface.h"
#include "UObject/ObjectMacros.h"
#include "ActionSystemTestPawn.generated.h"

class UNexusEventManagerComponent;

UCLASS(Blueprintable, BlueprintType, notplaceable)
class AActionSystemTestPawn : public APawn, public INexusActionInterface
{
	GENERATED_BODY()

public:
	AActionSystemTestPawn();
	virtual UNexusActionComponent* GetActionComponent() const override;
	virtual void PossessedBy(AController* NewController) override;

private:
	UPROPERTY()
	TObjectPtr<UNexusActionComponent> ActionComponent;
};
