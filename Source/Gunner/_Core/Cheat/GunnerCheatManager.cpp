// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerCheatManager.h"

#include "Gunner/_Core/GunnerTeamAgentInterface.h"
#include "GameFramework/GameModeBase.h"

void UGunnerCheatManager::InitCheatManager()
{
	Super::InitCheatManager();
	FGameModeEvents::GameModePostLoginEvent.AddUObject(this, &UGunnerCheatManager::OnPlayerPostLogin);
}

void UGunnerCheatManager::SetCheatTeamMode(ECheatTeamMode NewCheatTeamMode)
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

void UGunnerCheatManager::OnPlayerPostLogin(AGameModeBase* GameModeBase, APlayerController* PlayerController)
{
	static FGenericTeamId PingPongTeam = AttackerTeam;
	switch (CheatTeamMode)
	{
	case ECheatTeamMode::None:
		break;
	case ECheatTeamMode::EveryoneHostile:
		SetTeam(PlayerController, FGenericTeamId::NoTeam);
		break;
	case ECheatTeamMode::EveryoneFriendly:
		SetTeam(PlayerController, AttackerTeam);
		break;
	case ECheatTeamMode::PingPong:
		SetTeam(PlayerController, PingPongTeam);
		PingPongTeam = (PingPongTeam == AttackerTeam) ? DefenderTeam : AttackerTeam;
		break;
	default:
		break;
	}
}

void UGunnerCheatManager::SetAllControllersTeam(FGenericTeamId TeamId)
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

void UGunnerCheatManager::SetAllControllersTeamPingPong()
{
	static FGenericTeamId PingPongTeam = AttackerTeam;
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Iterator->Get();
		if (PC)
		{
			SetTeam(PC, PingPongTeam);
			PingPongTeam = (PingPongTeam == AttackerTeam) ? DefenderTeam : AttackerTeam;
		}
	}
}

void UGunnerCheatManager::SetTeam(APlayerController* PC, FGenericTeamId TeamId)
{
	if (IGunnerTeamAgentInterface* TeamAgentInterface = Cast<IGunnerTeamAgentInterface>(PC->PlayerState))
	{
		TeamAgentInterface->SetGenericTeamId(TeamId);
	}
}
