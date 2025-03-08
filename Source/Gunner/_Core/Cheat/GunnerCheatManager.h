// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NexusCheatManager.h"
#include "GunnerCheatManager.generated.h"

/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerCheatManager : public UNexusCheatManager
{
	GENERATED_BODY()
public:
	virtual void InitCheatManager() override;
	UFUNCTION(Exec)
	void ToggleEveryoneHostile();
	


private:
	void OnPlayerPostLogin(AGameModeBase* GameModeBase, APlayerController* PlayerController);
	void SetTeamNoTeam(APlayerController* PC);

private:
	bool bEveryoneHostile = false;
};
