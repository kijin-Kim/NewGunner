// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "GunnerCheatManager.generated.h"

/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerCheatManager : public UCheatManager
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
