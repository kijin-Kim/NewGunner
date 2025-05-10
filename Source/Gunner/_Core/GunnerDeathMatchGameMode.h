// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerGameMode.h"
#include "GunnerDeathMatchGameMode.generated.h"

class AGunnerSpawnPointActor;
/**
 * 
 */
UCLASS()
class GUNNER_API AGunnerDeathMatchGameMode : public AGunnerGameMode
{
	GENERATED_BODY()

public:
	AGunnerDeathMatchGameMode();
	virtual void AuthRegisterKill(AController* Killer, AController* Victim, FName KillCauserName) override;

protected:
	UPROPERTY(EditDefaultsOnly)
	float RespawnDelay = 3.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 KillLimit = 1;
};
