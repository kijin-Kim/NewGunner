// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerTeamAgentInterface.h"
#include "GameFramework/PlayerState.h"
#include "GunnerLobbyPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class GUNNER_API AGunnerLobbyPlayerState : public APlayerState, public IGunnerTeamAgentInterface
{
	GENERATED_BODY()

public:
	AGunnerLobbyPlayerState();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void CopyProperties(APlayerState* PlayerState) override;
	virtual void SetGenericTeamId(const FGenericTeamId& InTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamID; }
	virtual FOnGunnerTeamSetSignature* GetOnTeamSetDelegate() override { return &OnTeamSet; }

private:
	UFUNCTION()
	void OnRep_TeamID(FGenericTeamId OldTeamID);

public:
	UPROPERTY(ReplicatedUsing = OnRep_TeamID)
	FGenericTeamId TeamID = 0;

	FOnGunnerTeamSetSignature OnTeamSet;
};
