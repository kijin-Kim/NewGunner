// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerTeamDeathMatchGameMode.h"
#include "GunnerTeamDeathMatchGameState.h"
#include "Character/GunnerCharacter.h"
#include "Gunner/Gunner.h"

AGunnerTeamDeathMatchGameMode::AGunnerTeamDeathMatchGameMode()
{
	GameStateClass = AGunnerTeamDeathMatchGameState::StaticClass();
}

void AGunnerTeamDeathMatchGameMode::AuthRegisterKill(AController* Killer, AController* Victim, FName KillCauserName)
{
	Super::AuthRegisterKill(Killer, Victim, KillCauserName);
	if (!Victim)
	{
		return;
	}

	if (AGunnerCharacter* GunnerCharacter = Cast<AGunnerCharacter>(Victim->GetPawn()))
	{
		FTimerHandle RespawnTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, [this, Victim]()
		{
			if (!Victim)
			{
				return;
			}

			if (IGunnerTeamAgentInterface* TeamAgentInterface = Cast<IGunnerTeamAgentInterface>(Victim->GetPawn()))
			{
				const FGenericTeamId TeamID = TeamAgentInterface->GetGenericTeamId();
				const int32 Index = TeamID.GetId();
				if (AActor* PlayerStart = FindPlayerStart(Victim, FString::FromInt(Index)))
				{
					Victim->GetPawn()->SetActorLocation(PlayerStart->GetActorLocation());
				}
				RestartPlayer(Victim);
			}
		}, RespawnDelay, false);
		RespawnTimerHandles.Add(RespawnTimerHandle);
	}

	AGunnerTeamDeathMatchGameState* TdmGameState = GetGameState<AGunnerTeamDeathMatchGameState>();
	IGenericTeamAgentInterface* TeamAgentInterface = Cast<IGenericTeamAgentInterface>(Killer->GetPawn());
	if (TeamAgentInterface->GetTeamAttitudeTowards(*Victim->GetPawn()) == ETeamAttitude::Hostile)
	{
		TdmGameState->AddTeamKillCount(TeamAgentInterface->GetGenericTeamId(), 1);
	}
}

bool AGunnerTeamDeathMatchGameMode::ReadyToEndMatch_Implementation()
{
	if (bool bShouldEndMatch = Super::ReadyToEndMatch_Implementation())
	{
		return true;
	}

	AGunnerTeamDeathMatchGameState* TdmGameState = GetGameState<AGunnerTeamDeathMatchGameState>();
	for (const auto& KillCount : TdmGameState->GetKillCountPerTeam())
	{
		if (KillCount.Value >= KillCountToWin)
		{
			return true;
		}
	}

	return false;
}

void AGunnerTeamDeathMatchGameMode::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();
	for (FTimerHandle TimerHandle : RespawnTimerHandles)
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	}
	RespawnTimerHandles.Empty();
}
