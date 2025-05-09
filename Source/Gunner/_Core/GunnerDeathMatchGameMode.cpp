// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerDeathMatchGameMode.h"
#include "GunnerDeathMatchGameState.h"
#include "Character/GunnerCharacter.h"
#include "Gunner/Gunner.h"

AGunnerDeathMatchGameMode::AGunnerDeathMatchGameMode()
{
	GameStateClass = AGunnerDeathMatchGameState::StaticClass();
}

void AGunnerDeathMatchGameMode::AuthRegisterKill(AController* Killer, AController* Victim, FName KillCauserName)
{
	Super::AuthRegisterKill(Killer, Victim, KillCauserName);
	AGunnerGameState* GS = GetGameState<AGunnerGameState>();
	FGunnerKillInfo* KillInfo = GS->GetKillerInfo(Killer);
	if (KillInfo && KillInfo->Kills >= KillLimit)
	{
		EndMatch();
	}

	if (!Victim)
	{
		return;
	}

	if (AGunnerCharacter* GunnerCharacter = Cast<AGunnerCharacter>(Victim->GetPawn()))
	{
		GunnerCharacter->AuthRemoveActionSets();
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
	}
}
