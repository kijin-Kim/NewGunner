// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
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

UCLASS(minimalapi)
class AGunnerGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	

	UFUNCTION(BlueprintCallable)
	virtual void AuthRegisterKill(AController* Killer, AController* Victim, FName KillCauserName);
	virtual bool ReadyToEndMatch_Implementation() override;
	virtual void HandleMatchHasEnded() override;


#if WITH_EDITORONLY_DATA
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EExclusivePawnSpawnMode ExclusivePawnSpawnMode = EExclusivePawnSpawnMode::None;

private:
	bool bSpawnedFirstClient = false;
#endif
	
};
