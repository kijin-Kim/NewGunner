// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NexusPlayerState.h"
#include "Gunner/_Core/GunnerTeamAgentInterface.h"
#include "GunnerPlayerState.generated.h"


class UNexusCueComponent;
class UNexusGameplayTagComponent;
class UNexusPropertyComponent;
class UNexusSideEffectComponent;
class UNexusPredictionComponent;
class UGunnerActionComponent;
class UGunnerInventoryManagerComponent;
class UNexusEventManagerComponent;
class UNexusActionComponent;
/**
 * 
 */
UCLASS()
class GUNNER_API AGunnerPlayerState : public ANexusPlayerState, public IGunnerTeamAgentInterface
{
	GENERATED_BODY()

public:
	AGunnerPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetGenericTeamId(const FGenericTeamId& InTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamID; }
	virtual FOnGunnerTeamSetSignature* GetOnTeamSetDelegate() override { return &OnTeamSet; }

private:
	UFUNCTION()
	void OnPawnSetEvent(APlayerState* Player, APawn* NewPawn, APawn* OldPawn);
	UFUNCTION()
	void OnRep_TeamID(FGenericTeamId OldTeamID);

private:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGunnerInventoryManagerComponent> InventoryManagerComponent;


	UPROPERTY(ReplicatedUsing = OnRep_TeamID)
	FGenericTeamId TeamID = 0;

	FOnGunnerTeamSetSignature OnTeamSet;
};
