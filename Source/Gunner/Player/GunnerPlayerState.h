// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Gunner/_Core/GunnerTeamAgentInterface.h"
#include "GunnerPlayerState.generated.h"


class UNexusPredictionComponent;
class UGunnerActionComponent;
class UGunnerSlotManagerComponent;
class UNexusEventManagerComponent;
class UNexusActionComponent;
/**
 * 
 */
UCLASS()
class GUNNER_API AGunnerPlayerState : public APlayerState, public IGunnerTeamAgentInterface
{
	GENERATED_BODY()

public:
	AGunnerPlayerState();

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
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = true))
	TObjectPtr<UGunnerActionComponent> ActionComponent;
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = true))
	TObjectPtr<UNexusPredictionComponent> PredictionComponent;
	UPROPERTY()
	TObjectPtr<UNexusEventManagerComponent> EventManagerComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGunnerSlotManagerComponent> SlotManagerComponent;
	

	UPROPERTY(ReplicatedUsing = OnRep_TeamID)
	FGenericTeamId TeamID = 0;

	FOnGunnerTeamSetSignature OnTeamSet;
};
