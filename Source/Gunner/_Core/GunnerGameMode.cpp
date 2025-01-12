// Copyright Epic Games, Inc. All Rights Reserved.

#include "GunnerGameMode.h"

#include "GunnerGameState.h"
#include "Gunner/Gunner.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"


void AGunnerGameMode::AuthRegisterKill(AController* Killer, AController* Victim)
{
	GetGameState<AGunnerGameState>()->AuthRegisterKill(Killer, Victim);
}

bool AGunnerGameMode::ReadyToEndMatch_Implementation()
{
	return GetWorld()->GetTimeSeconds() >= GetGameState<AGunnerGameState>()->GetMatchTimeLimit();
}

void AGunnerGameMode::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.1f);
}
