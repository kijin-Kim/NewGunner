// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerTestGameMode.h"

#include "GunnerTestPawn.h"

AGunnerTestGameMode::AGunnerTestGameMode()
{
	DefaultPawnClass = AGunnerTestPawn::StaticClass();
}
