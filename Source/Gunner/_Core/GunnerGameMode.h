// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"

#include "GunnerGameMode.generated.h"

class AGunnerGameState;

UCLASS(minimalapi)
class AGunnerGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	virtual void AuthRegisterKill(AController* Killer, AController* Victim, FName KillCauserName);
	virtual bool ReadyToEndMatch_Implementation() override;
	virtual void HandleMatchHasEnded() override;
	
};
