// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GunnerGameState.generated.h"


USTRUCT()
struct FGunnerKillInfo
{
	GENERATED_BODY()

	FGunnerKillInfo()
		: KillerPlayerId(-1),
		  Kills(0)
	{
	}

	UPROPERTY()
	int32 KillerPlayerId;
	UPROPERTY()
	int32 Kills;
};

USTRUCT(BlueprintType)
struct FGunnerKillLog
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	APlayerState* KillerPlayerState;
	UPROPERTY(BlueprintReadOnly)
	APlayerState* VictimPlayerState;
	UPROPERTY(BlueprintReadOnly)
	FName KillCauserName;

	FString ToString() const;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGunnerMatchEndedSignature, const TArray<int32>&, WinnerIds);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGunnerNewKillConfirmedSignature, const FGunnerKillLog&, KillLog);




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
	void AuthRegisterKill(AController* Killer, AController* Victim, FName KillCauserName);
	FGunnerKillInfo* GetKillerInfo(AController* Killer);

	virtual void HandleMatchHasEnded() override;
	virtual TArray<int32> DetermineWinners() const { return {}; }

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticastBroadcastWinners(const TArray<int32>& WinnerIds);
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticastBroadcastKill(const FGunnerKillLog& KillLog);



public:
	UPROPERTY(BlueprintAssignable)
	FOnGunnerMatchEndedSignature OnMatchEndedDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnGunnerNewKillConfirmedSignature OnNewKillConfirmedDelegate;

protected:
	UPROPERTY(Replicated)
	TArray<FGunnerKillInfo> KillInfos;

private:
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	double MatchTimeLimit = 40.0f;
	
};
