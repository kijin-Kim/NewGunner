// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GunnerGameState.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGunnerMatchEnded, const TArray<int32>&, WinnerIds);


USTRUCT()
struct FGunnerPlayerKillInfo
{
	GENERATED_BODY()

	UPROPERTY()
	int32 PlayerId;
	UPROPERTY()
	int32 Kills;
};

/**
 * 
 */
UCLASS()
class GUNNER_API AGunnerGameState : public AGameState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetMatchTimeLimit(double NewTimeLimit) { MatchTimeLimit = NewTimeLimit; }
	double GetMatchTimeLimit() const { return MatchTimeLimit; }
	void AuthRegisterKill(AController* Killer, AController* Victim);
	FGunnerPlayerKillInfo* GetKillerInfo(AController* Killer);

	virtual void HandleMatchHasEnded() override;
	virtual TArray<int32> DetermineWinners() const { return {}; }
	
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticastBroadcastWinners(const TArray<int32>& WinnerIds);

public:
	UPROPERTY(BlueprintAssignable)
	FOnGunnerMatchEnded OnMatchEndedDelegate;

protected:
	UPROPERTY(Replicated)
	TArray<FGunnerPlayerKillInfo> PlayerKills;

private:
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	double MatchTimeLimit = 40.0f;
};
