// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerGameMode.h"
#include "GunnerTeamDeathMatchGameMode.generated.h"

class AGunnerSpawnPointActor;



/**
 * 
 */
UCLASS()
class GUNNER_API AGunnerTeamDeathMatchGameMode : public AGunnerGameMode
{
	GENERATED_BODY()

public:
	AGunnerTeamDeathMatchGameMode();
	virtual void AuthRegisterKill(AController* Killer, AController* Victim, FName KillCauserName) override;
	virtual bool ReadyToEndMatch_Implementation() override;
	virtual void HandleMatchHasEnded() override;


protected:
	UPROPERTY(EditDefaultsOnly)
	float RespawnDelay = 3.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 KillCountToWin = 1;

private:
	TArray<FTimerHandle> RespawnTimerHandles;
};
