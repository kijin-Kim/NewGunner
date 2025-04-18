#pragma once

#include "CoreMinimal.h"
#include "NexusActionInterface.h"
#include "UObject/ObjectMacros.h"
#include "NexusActionTestPawn.generated.h"

class UNexusPredictionComponent;
class UNexusSideEffectComponent;
class UNexusGameplayTagComponent;
class UNexusPropertyComponent;
class UNexusCueComponent;
class UNexusEventManagerComponent;

UCLASS(Blueprintable, BlueprintType, notplaceable)
class ANexusActionTestPawn : public APawn, public INexusActionInterface
{
	GENERATED_BODY()

public:
	ANexusActionTestPawn();
	virtual UNexusActionComponent* GetActionComponent() const override;
	virtual void NotifyControllerChanged() override;
	

private:
	UPROPERTY()
	TObjectPtr<UNexusActionComponent> ActionComponent;
	UPROPERTY()
	TObjectPtr<UNexusCueComponent> CueComponent;
	UPROPERTY()
	TObjectPtr<UNexusPropertyComponent> PropertyComponent;
	UPROPERTY()
	TObjectPtr<UNexusGameplayTagComponent> GameplayTagComponent;
	UPROPERTY()
	TObjectPtr<UNexusSideEffectComponent> SideEffectComponent;
	UPROPERTY()
	TObjectPtr<UNexusPredictionComponent> PredictionComponent;
	UPROPERTY()
	TObjectPtr<UNexusEventManagerComponent> EventManagerComponent;
};
