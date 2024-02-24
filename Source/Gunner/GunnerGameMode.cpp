// Copyright Epic Games, Inc. All Rights Reserved.

#include "GunnerGameMode.h"
#include "GunnerCharacter.h"
#include "UObject/ConstructorHelpers.h"

AGunnerGameMode::AGunnerGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
