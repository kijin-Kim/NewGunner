// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerPlayerState.h"

#include "Gunner/Core/ActionSystem/GunnerAction.h"
#include "Gunner/Core/ActionSystem/GunnerActionComponent.h"
#include "Gunner/Core/Event/EventManagerComponent.h"

AGunnerPlayerState::AGunnerPlayerState()
{
	NetUpdateFrequency = 100.0f;
	ActionComponent = CreateDefaultSubobject<UGunnerActionComponent>(TEXT("ActionComponent"));
	EventManagerComponent = CreateDefaultSubobject<UEventManagerComponent>(TEXT("EventManagerComponent"));
}

void AGunnerPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (HasAuthority())
	{
		SetupOnPossessedPawnChangedEvent();
	}
}

void AGunnerPlayerState::ClientInitialize(AController* C)
{
	Super::ClientInitialize(C);
	SetupOnPossessedPawnChangedEvent();
}

void AGunnerPlayerState::SetupOnPossessedPawnChangedEvent()
{
	if (AController* Controller = GetOwningController())
	{
		Controller->OnPossessedPawnChanged.AddUniqueDynamic(this, &ThisClass::OnPossessedPawnChanged);
		if (APawn* PossessedPawn = Controller->GetPawn())
		{
			OnPossessedPawnChanged(nullptr, PossessedPawn);
		}
	}
}

void AGunnerPlayerState::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	if (NewPawn)
	{
		ActionComponent->InitActionComponent(this, NewPawn);

		if (HasAuthority() && TestActionClass)
		{
			FGunnerActionDefinition ActionDefinition(this, TestActionClass);
			ActionComponent->AddAction(ActionDefinition);
		}
	}
}
