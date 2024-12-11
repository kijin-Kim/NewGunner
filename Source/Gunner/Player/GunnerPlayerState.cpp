// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerPlayerState.h"

#include "Gunner/_Core/ActionSystem/GunnerActionComponent.h"
#include "Gunner/_Core/Event/GunnerEventManagerComponent.h"

AGunnerPlayerState::AGunnerPlayerState()
{
	NetUpdateFrequency = 100.0f;
	ActionComponent = CreateDefaultSubobject<UGunnerActionComponent>(TEXT("ActionComponent"));
	EventManagerComponent = CreateDefaultSubobject<UGunnerEventManagerComponent>(TEXT("EventManagerComponent"));
}

void AGunnerPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	OnPawnSet.AddDynamic(this, &AGunnerPlayerState::OnPawnSetEvent);
}

void AGunnerPlayerState::OnPawnSetEvent(APlayerState* Player, APawn* NewPawn, APawn* OldPawn)
{
	if (!NewPawn && OldPawn && HasAuthority())
	{
		ActionComponent->AuthRemoveAllActions();
		ActionComponent->AuthRemoveAllProperties();
		EventManagerComponent->UnbindAllEventCallbacks();
	}
}
