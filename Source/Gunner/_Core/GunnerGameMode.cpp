#include "GunnerGameMode.h"

#include "GunnerGameState.h"
#include "GameFramework/PlayerState.h"
#include "Gunner/Gunner.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"


void AGunnerGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
#if !UE_BUILD_SHIPPING
	bSpawnedFirstClient = false;
#endif
}

void AGunnerGameMode::RestartPlayer(AController* NewPlayer)
{
#if UE_BUILD_SHIPPING
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
}

void AGunnerGameMode::AuthRegisterKill(AController* Killer, AController* Victim, FName KillCauserName)
{
	GetGameState<AGunnerGameState>()->AuthRegisterKill(Killer, Victim, KillCauserName);
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
