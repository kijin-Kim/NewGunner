// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerPlayerState.h"

#include "Gunner/Gunner.h"
#include "Gunner/Core/ActionSystem/GunnerAction.h"
#include "Gunner/Core/ActionSystem/GunnerActionComponent.h"
#include "Gunner/Core/Event/GunnerEventManagerComponent.h"

AGunnerPlayerState::AGunnerPlayerState()
{
	NetUpdateFrequency = 100.0f;
	ActionComponent = CreateDefaultSubobject<UGunnerActionComponent>(TEXT("ActionComponent"));
	EventManagerComponent = CreateDefaultSubobject<UGunnerEventManagerComponent>(TEXT("EventManagerComponent"));
}

void AGunnerPlayerState::PreNetReceive()
{
	Super::PreNetReceive();
	if (GetWorld())
	{
		GR_LOG(LogGunner, Warning, TEXT(" [%f]"), GetWorld()->GetTimeSeconds());
	}
}

void AGunnerPlayerState::PostNetReceive()
{
	Super::PostNetReceive();
	if (GetWorld())
	{
		GR_LOG(LogGunner, Warning, TEXT(" [%f]"), GetWorld()->GetTimeSeconds());
	}
}
