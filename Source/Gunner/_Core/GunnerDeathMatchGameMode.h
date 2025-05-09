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
	UPROPERTY()
	float RespawnDelay = 3.0f;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 KillLimit = 1;
};
