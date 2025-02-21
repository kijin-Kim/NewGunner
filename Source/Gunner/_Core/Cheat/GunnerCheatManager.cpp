// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerCheatManager.h"

#include "Gunner/_Core/GunnerTeamAgentInterface.h"
#include "GameFramework/GameModeBase.h"

void UGunnerCheatManager::InitCheatManager()
{
	Super::InitCheatManager();
	FGameModeEvents::GameModePostLoginEvent.AddUObject(this, &UGunnerCheatManager::OnPlayerPostLogin);
}

void UGunnerCheatManager::ToggleEveryoneHostile()
{
	bEveryoneHostile = !bEveryoneHostile;

	UWorld* World = GetWorld();
	if (!World && !bEveryoneHostile)
	{
		return;
	}

	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Iterator->Get();
		if (PC)
		{
			SetTeamNoTeam(PC);
		}
	}
}

void UGunnerCheatManager::OnPlayerPostLogin(AGameModeBase* GameModeBase, APlayerController* PlayerController)
{
	if (bEveryoneHostile)
	{
		SetTeamNoTeam(PlayerController);
	}
}

void UGunnerCheatManager::SetTeamNoTeam(APlayerController* PC)
{
	if (IGunnerTeamAgentInterface* TeamAgentInterface = Cast<IGunnerTeamAgentInterface>(PC->PlayerState))
	{
		TeamAgentInterface->SetGenericTeamId(FGenericTeamId::NoTeam);
	}
}
