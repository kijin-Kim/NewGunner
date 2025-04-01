// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "NexusCheatManager.h"
#include "GunnerCheatManager.generated.h"


UENUM()
enum class ECheatTeamMode
{
	None,
	EveryoneHostile,
	EveryoneFriendly,
	PingPong,
};

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
	void SetCheatTeamMode(ECheatTeamMode NewCheatTeamMode);


private:
	void OnPlayerPostLogin(AGameModeBase* GameModeBase, APlayerController* PlayerController);
	void SetAllControllersTeam(FGenericTeamId TeamId);
	void SetAllControllersTeamPingPong();
	void SetTeam(APlayerController* PC, FGenericTeamId TeamId);

private:
	UPROPERTY()
	ECheatTeamMode CheatTeamMode = ECheatTeamMode::None;
};
