#pragma once

#include "CoreMinimal.h"
#include "NexusActionInterface.h"
#include "Gunner/Action/GunnerActionDropSlotItem.h"
#include "Gunner/Slot/GunnerSlotManagerInterface.h"
#include "Gunner/_Core/GunnerSlotItemPickup.h"
#include "Gunner/_Core/GunnerTeamAgentInterface.h"
#include "UObject/ObjectMacros.h"
#include "GunnerTestPawn.generated.h"

class UGunnerTestActionComponent;
class UNexusPredictionComponent;
class UNexusSideEffectComponent;
class UNexusGameplayTagComponent;
class UNexusPropertyComponent;
class UNexusCueComponent;
class UGunnerTestSlotManagerComponent;
class UNexusEventManagerComponent;


UCLASS()
class AGunnerTestSlotItemPickup : public AGunnerSlotItemPickup
{
	GENERATED_BODY()

public:
	 AGunnerTestSlotItemPickup();
};


UCLASS()
class UGunnerActionTestDropSlotItem : public UGunnerActionDropSlotItem
{
	GENERATED_BODY()

public:
	UGunnerActionTestDropSlotItem();
};


UCLASS()
class AGunnerTestPawn : public APawn, public INexusActionInterface, public IGunnerSlotManagerInterface, public IGunnerTeamAgentInterface
{
	GENERATED_BODY()

public:
	AGunnerTestPawn();
	virtual UNexusActionComponent* GetActionComponent() const override;
	virtual UGunnerSlotManagerComponent* GetSlotManagerComponent() const override;
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamID; }
	virtual FOnGunnerTeamSetSignature* GetOnTeamSetDelegate() override { return &OnTeamSet; }

	virtual void NotifyControllerChanged() override;

public:
	FGenericTeamId TeamID = 0;
	FOnGunnerTeamSetSignature OnTeamSet;

private:
	UPROPERTY()
	TObjectPtr<UGunnerTestActionComponent> ActionComponent;
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


	UPROPERTY()
	TObjectPtr<UGunnerSlotManagerComponent> SlotManagerComponent;
};
