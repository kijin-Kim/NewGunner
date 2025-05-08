// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GunnerTeamAgentInterface.h"
#include "GameFramework/GameMode.h"

#include "GunnerGameMode.generated.h"

class AGunnerGameState;


#if WITH_EDITORONLY_DATA
UENUM(BlueprintType)
enum class EExclusivePawnSpawnMode : uint8
{
	None,
	SpawnListenServerExclusively,
	SpawnFirstClientExclusively
};
#endif


UENUM()
enum class ECheatTeamMode
{
	None,
	EveryoneHostile,
	EveryoneFriendly,
	PingPong,
};

UCLASS(minimalapi)
class AGunnerGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	UFUNCTION(Exec)
	void SetCheatTeamMode(ECheatTeamMode NewCheatTeamMode);
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	virtual void AuthRegisterKill(AController* Killer, AController* Victim, FName KillCauserName);
	virtual bool ReadyToEndMatch_Implementation() override;
	virtual void HandleMatchHasEnded() override;

private:
	void SetAllControllersTeam(FGenericTeamId TeamId);
	void SetAllControllersTeamPingPong();
	void SetTeam(AController* Controller, FGenericTeamId TeamID);



#if WITH_EDITORONLY_DATA
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EExclusivePawnSpawnMode ExclusivePawnSpawnMode = EExclusivePawnSpawnMode::None;

private:
	bool bSpawnedFirstClient = false;
#endif
	ECheatTeamMode CheatTeamMode = ECheatTeamMode::None;
	FGenericTeamId PingPongTeamID = AttackerTeam;
};
