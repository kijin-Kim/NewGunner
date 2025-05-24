// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerGameMode.h"
#include "GameFramework/GameState.h"
#include "GunnerGameState.generated.h"


class UCanvasRenderTarget2D;

USTRUCT()
struct FGunnerKillInfo
{
	GENERATED_BODY()

	FGunnerKillInfo()
		: TeamId(FGenericTeamId::NoTeam),
		  KillerPlayerId(-1),
		  Kills(0)
	{
	}

	UPROPERTY()
	FGenericTeamId TeamId;
	UPROPERTY()
	int32 KillerPlayerId;
	UPROPERTY()
	int32 Kills;
};

USTRUCT(BlueprintType)
struct FGunnerKillFeed
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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGunnerNewKillConfirmedSignature, const FGunnerKillFeed&, KillLog);


USTRUCT(BlueprintType)
struct FGunnerFogOfWarRenderTargets
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UCanvasRenderTarget2D> VisionConeRenderTarget;
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UCanvasRenderTarget2D> InformationRenderTarget;
};


/**
 * 
 */
UCLASS()
class GUNNER_API AGunnerGameState : public AGameState
{
	GENERATED_BODY()

public:
	AGunnerGameState();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticastBroadcastWinners(const TArray<int32>& WinnerIds);
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticastBroadcastKill(const FGunnerKillFeed& KillLog);
	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchHasEnded() override;

	void SetMatchTimeLimitSeconds(double Seconds) { ServerMatchTimeLimitSeconds = Seconds; }

	UFUNCTION()
	virtual void OnRep_KillInfos();
	void UpdateKillInfos(AController* Killer);

	const TArray<FGunnerKillInfo>& GetKillInfos() const { return KillInfos; }

	UFUNCTION(BlueprintCallable)
	virtual const FGunnerFogOfWarRenderTargets& FindOrAddPlayerFogOfWarRenderTargets(int32 PlayerID);

public:
	UPROPERTY(BlueprintAssignable)
	FOnGunnerMatchEndedSignature OnMatchEndedDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnGunnerNewKillConfirmedSignature OnNewKillConfirmedDelegate;

protected:
	UPROPERTY(BlueprintReadOnly)
	TMap<int32, FGunnerFogOfWarRenderTargets> PlayerFogOfWarRenderTargets;

private:
	UPROPERTY(ReplicatedUsing = OnRep_KillInfos)
	TArray<FGunnerKillInfo> KillInfos;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	double ServerMatchTimeLimitSeconds = 40.0f;
};
