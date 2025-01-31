// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerPlayerState.h"

#include "Gunner/_Core/ActionSystem/GunnerActionComponent.h"
#include "Gunner/_Core/Event/GunnerEventManagerComponent.h"
#include "Net/UnrealNetwork.h"


AGunnerPlayerState::AGunnerPlayerState()
{
	NetUpdateFrequency = 100.0f;
	ActionComponent = CreateDefaultSubobject<UGunnerActionComponent>(TEXT("ActionComponent"));
	EventManagerComponent = CreateDefaultSubobject<UGunnerEventManagerComponent>(TEXT("EventManagerComponent"));
}

void AGunnerPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (OnPawnSet.IsBound())
	{
		OnPawnSet.RemoveDynamic(this, &AGunnerPlayerState::OnPawnSetEvent);
	}
	OnPawnSet.AddDynamic(this, &AGunnerPlayerState::OnPawnSetEvent);
}

void AGunnerPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGunnerPlayerState, TeamID);
}

void AGunnerPlayerState::SetGenericTeamId(const FGenericTeamId& InTeamID)
{
	TeamID = InTeamID;
}

void AGunnerPlayerState::OnPawnSetEvent(APlayerState* Player, APawn* NewPawn, APawn* OldPawn)
{
	if (!HasAuthority())
	{
		if (NewPawn)
		{
			ActionComponent->InitActionComponent(NewPawn);
		}
		return;
	}

	if (NewPawn && NewPawn != OldPawn)
	{
		ActionComponent->ReleaseActionComponent();
		ActionComponent->InitActionComponent(NewPawn);
	}
	else if (!NewPawn)
	{
		ActionComponent->ReleaseActionComponent();
		EventManagerComponent->UnbindAllEventCallbacks();
	}
}
