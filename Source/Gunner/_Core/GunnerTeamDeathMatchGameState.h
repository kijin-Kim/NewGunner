// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerGameState.h"
#include "GunnerTeamDeathMatchGameState.generated.h"


DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGunnerTeamKillCountChangedSignature, FGenericTeamId /*TeamId*/, int32 /*KillCount*/);

/**
 * 
 */
UCLASS()
class GUNNER_API AGunnerTeamDeathMatchGameState : public AGunnerGameState
{
	GENERATED_BODY()

public:
	void AddTeamKillCount(FGenericTeamId TeamId, int32 KillCount);


	virtual void OnRep_KillInfos() override;
	virtual void HandleMatchHasEnded() override;
	const TMap<FGenericTeamId, int32>& GetKillCountPerTeam() const { return KillCountPerTeam; }
	virtual const FGunnerFogOfWarRenderTargets& FindOrAddPlayerFogOfWarRenderTargets(int32 PlayerId) override;


public:
	FOnGunnerTeamKillCountChangedSignature OnTeamKillCountChanged;

private:
	TMap<FGenericTeamId, int32> KillCountPerTeam;
};
