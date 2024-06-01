// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnManager.h"
#include "GameFramework/GameModeBase.h"


ASpawnManager::ASpawnManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASpawnManager::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	FGameModeEvents::GameModePostLoginEvent.AddUObject(this, &ThisClass::OnPostLogin);
}

void ASpawnManager::OnPostLogin(AGameModeBase* GameModeBase, APlayerController* PlayerController)
{
}




