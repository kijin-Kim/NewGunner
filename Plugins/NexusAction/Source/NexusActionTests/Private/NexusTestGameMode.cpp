// Fill out your copyright notice in the Description page of Project Settings.


#include "NexusTestGameMode.h"

#include "NexusActionTestPawn.h"

ANexusTestGameMode::ANexusTestGameMode()
{
	DefaultPawnClass = ANexusActionTestPawn::StaticClass();
}
