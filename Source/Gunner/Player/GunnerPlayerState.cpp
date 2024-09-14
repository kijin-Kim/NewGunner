// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerPlayerState.h"
#include "Gunner/Core/ActionSystem/GunnerAction.h"
#include "Gunner/Core/ActionSystem/GunnerActionComponent.h"
#include "Gunner/Core/ActionSystem/TestGunnerAction.h"

AGunnerPlayerState::AGunnerPlayerState()
{
	NetUpdateFrequency = 100.0f;
	ActionComponent = CreateDefaultSubobject<UGunnerActionComponent>(TEXT("ActionComponent"));
}

void AGunnerPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (HasAuthority())
	{
		SetupActionComponent();
	};
}

void AGunnerPlayerState::BeginPlay()
{
	Super::BeginPlay();
}

void AGunnerPlayerState::ClientInitialize(AController* C)
{
	Super::ClientInitialize(C);
	SetupActionComponent();
}

void AGunnerPlayerState::SetupActionComponent()
{
	ActionComponent->InitActionComponent(this, GetPawn());
	if (HasAuthority())
	{
		FGunnerActionDefinition ActionDefinition(this, UTestGunnerAction::StaticClass());
		ActionComponent->AddAction(ActionDefinition);
	}

	FTimerHandle Temp;
	GetWorld()->GetTimerManager().SetTimer(Temp, [this]()
	{
		ActionComponent->TEST_TRIGGER_ACTIONS();
	}, 5.0f, false);
}
