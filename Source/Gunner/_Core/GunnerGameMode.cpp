#include "GunnerGameMode.h"

#include "GenericTeamAgentInterface.h"
#include "GunnerGameState.h"
#include "GunnerTeamAgentInterface.h"
#include "GameFramework/PlayerState.h"
#include "Gunner/Gunner.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"


void AGunnerGameMode::SetCheatTeamMode(ECheatTeamMode NewCheatTeamMode)
{
	CheatTeamMode = NewCheatTeamMode;

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	switch (CheatTeamMode)
	{
	case ECheatTeamMode::None:
		break;
	case ECheatTeamMode::EveryoneHostile:
		SetAllControllersTeam(FGenericTeamId::NoTeam);
		break;
	case ECheatTeamMode::EveryoneFriendly:
		SetAllControllersTeam(AttackerTeam);
		break;
	case ECheatTeamMode::PingPong:
		SetAllControllersTeamPingPong();
		break;
	default:
		break;
	}
}

void AGunnerGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
#if WITH_EDITOR
	bSpawnedFirstClient = false;
#endif
}

void AGunnerGameMode::RestartPlayer(AController* NewPlayer)
{
#if !WITH_EDITOR
	Super::RestartPlayer(NewPlayer);
#else

	switch (ExclusivePawnSpawnMode)
	{
	case EExclusivePawnSpawnMode::None:
		Super::RestartPlayer(NewPlayer);
		break;
	case EExclusivePawnSpawnMode::SpawnListenServerExclusively:
		if (NewPlayer->IsLocalController())
		{
			Super::RestartPlayer(NewPlayer);
		}
		break;
	case EExclusivePawnSpawnMode::SpawnFirstClientExclusively:
		if (!NewPlayer->IsLocalController() && !bSpawnedFirstClient)
		{
			bSpawnedFirstClient = true;
			Super::RestartPlayer(NewPlayer);
			break;
		}
		break;
	default:
		checkNoEntry();
		break;
	}
#endif


	switch (CheatTeamMode)
	{
	case ECheatTeamMode::None:
		break;
	case ECheatTeamMode::EveryoneHostile:
		SetTeam(NewPlayer, FGenericTeamId::NoTeam);
		break;
	case ECheatTeamMode::EveryoneFriendly:
		SetTeam(NewPlayer, AttackerTeam);
		break;
	case ECheatTeamMode::PingPong:
		SetAllControllersTeamPingPong();
		break;
	default:
		break;
	}
}

void AGunnerGameMode::AuthRegisterKill(AController* Killer, AController* Victim, FName KillCauserName)
{
	AGunnerGameState* GunnerGameState = GetGameState<AGunnerGameState>();

	if (!HasAuthority())
	{
		GR_LOG(LogGunner, Error, TEXT("권한 없는 함수 호출"));
		return;
	}

	if (!Killer || !Victim)
	{
		return;
	}

	FGunnerKillFeed KillFeed;
	KillFeed.KillerPlayerState = Killer->PlayerState;
	KillFeed.VictimPlayerState = Victim->PlayerState;
	KillFeed.KillCauserName = KillCauserName;
	GunnerGameState->NetMulticastBroadcastKill(KillFeed);


	GunnerGameState->UpdateKillInfos(Killer);

}

bool AGunnerGameMode::ReadyToEndMatch_Implementation()
{
	return GetGameState<AGunnerGameState>()->ElapsedTime >= MatchTimeLimitSeconds;
}

void AGunnerGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	AGunnerGameState* GunnerGameState = GetGameState<AGunnerGameState>();
	GunnerGameState->SetMatchTimeLimitSeconds(MatchTimeLimitSeconds);
}

void AGunnerGameMode::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.1f);
	AGunnerGameState* GunnerGameState = GetGameState<AGunnerGameState>();

	TArray<int32> WinnerIDs = DetermineWinners();
	GunnerGameState->NetMulticastBroadcastWinners(WinnerIDs);
}

TArray<int32> AGunnerGameMode::DetermineWinners() const
{
	int32 MaxKills = 0;
	int32 WinnerID = 0;
	AGunnerGameState* GunnerGameState = GetGameState<AGunnerGameState>();
	for (const FGunnerKillInfo& Info : GunnerGameState->GetKillInfos())
	{
		if (Info.Kills > MaxKills)
		{
			MaxKills = Info.Kills;
			WinnerID = Info.KillerPlayerId;
		}
	}
	return {WinnerID};
}

void AGunnerGameMode::SetAllControllersTeam(FGenericTeamId TeamId)
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Iterator->Get();
		if (PC)
		{
			SetTeam(PC, TeamId);
		}
	}
}

void AGunnerGameMode::SetAllControllersTeamPingPong()
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Iterator->Get();
		if (PC)
		{
			SetTeam(PC, Iterator.GetIndex() % 2 == 0 ? AttackerTeam : DefenderTeam);
		}
	}
}

void AGunnerGameMode::SetTeam(AController* Controller, FGenericTeamId TeamID)
{
	IGunnerTeamAgentInterface* TeamAgentInterface = Cast<IGunnerTeamAgentInterface>(Controller->PlayerState);
	if (ensure(TeamAgentInterface))
	{
		TeamAgentInterface->SetGenericTeamId(TeamID);
	}
}
